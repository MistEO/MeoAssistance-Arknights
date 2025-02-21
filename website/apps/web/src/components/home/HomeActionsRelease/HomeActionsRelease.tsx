import { GlowButton } from '@/components/foundations/GlowButton/GlowButton'
import { Release, useRelease } from '@/hooks/use-release'
import mdiAlertCircle from '@iconify/icons-mdi/alert-circle'
import mdiCheck from '@iconify/icons-mdi/check'
import mdiDownload from '@iconify/icons-mdi/download'
import mdiLoading from '@iconify/icons-mdi/loading'
import type { IconifyIcon } from '@iconify/react'
import { Icon } from '@iconify/react'

import clsx from 'clsx'
import { AnimatePresence, motion } from 'framer-motion'
import {
  Component,
  FC,
  ReactNode,
  forwardRef,
  useCallback,
  useEffect,
  useMemo,
  useState,
} from 'react'
import { useMount } from 'react-use'

import { downloadBlob } from '../../../utils/blob'
import {
  checkUrlConnectivity,
  checkUrlSpeed,
  download,
} from '../../../utils/fetch'
import { formatBytes } from '../../../utils/format'
import sleep from '../../../utils/sleep'
import {
  DetectionFailedSymbol,
  PLATFORMS,
  ResolvedPlatform,
  detectPlatform,
} from './ReleaseModels'

type GITHUB_MIRROR_TYPE = {
  name: string
  transform: (original: URL) => string
}

const GITHUB_MIRRORS: GITHUB_MIRROR_TYPE[] = [
  {
    name: 'origin',
    transform: (original: URL) => original.toString(),
  },
]

const DataLoadRate: FC<{ loaded: number; total: number }> = ({
  loaded,
  total,
}) => {
  const percentage = useMemo(() => {
    const percentage = (loaded / total) * 100
    return percentage > 100 ? 100 : percentage
  }, [loaded, total])

  return (
    <div className="flex flex-row items-center justify-center gap-2 font-mono">
      <div className="flex flex-col items-start justify-center gap-1">
        <div className="text-sm">{percentage.toFixed(0)}%</div>
        <div className="w-12 h-1 bg-white/10 rounded-full">
          <div
            className="h-full rounded-full bg-white"
            style={{ width: `${percentage}%` }}
          />
        </div>
      </div>
      <div className="flex flex-col items-end justify-center">
        <div className="text-sm">{formatBytes(loaded, 1)}</div>
        <div className="text-sm">{formatBytes(total, 1)}</div>
      </div>
    </div>
  )
}

interface DownloadStateProps {
  icon: IconifyIcon
  iconClassName?: string
  title: ReactNode
  className?: string
}

export const DownloadState: FC<DownloadStateProps> = forwardRef<
  HTMLDivElement,
  DownloadStateProps
>(({ icon, iconClassName, title, className }, ref) => {
  return (
    <motion.div
      className={clsx(
        'flex py-6 px-3 flex-col items-center justify-center text-white font-normal',
        className,
      )}
      {...{
        exit: {
          scale: 0.9,
          opacity: 0,
        },
        initial: {
          scale: 0,
          opacity: 0,
        },
        animate: {
          scale: 1,
          opacity: 1,
        },
        transition: {
          type: 'spring',
          stiffness: 500,
          damping: 30,
        },
      }}
      ref={ref}
    >
      <div className="flex items-center -ml-1">
        <Icon className={iconClassName} icon={icon} fontSize="28px" />
        <span className="ml-2">{title}</span>
      </div>
    </motion.div>
  )
})

type DownloadDetectionStates =
  | {
      state: 'idle'
    }
  | {
      state: 'detecting'
      detected: number
    }
  | {
      state: 'speedTesting'
      mirrorIndex: number
    }
  | {
      state: 'detected'
      availableMirror: number
      canTestSpeed: boolean
      cantTestSpeedReason: 'saveData' | 'mobile' | 'ok'
    }
  | {
      state: 'connecting'
      mirrorIndex: number
      mirrorLatency: number
      mirrorSpeed: number
    }
  | {
      state: 'downloading'
      mirrorIndex: number
      progressDownloaded: number
      progressTotal: number
    }
  | {
      state: 'downloaded'
    }
  | {
      state: 'fallback'
    }

const DownloadButton: FC<{
  platform: ResolvedPlatform
  releaseName: string | null
}> = ({ platform, releaseName }) => {
  const href = platform.asset.browser_download_url

  const [loadState, setLoadState] = useState<DownloadDetectionStates>({
    state: 'idle',
  })
  const mirrorsTemplate = [
    ...platform.asset.mirrors.map((url) => ({
      transform: () => url,
      name: new URL(url).hostname,
    })),
    ...GITHUB_MIRRORS,
  ]

  const detectDownload = useCallback(async () => {
    setLoadState({ state: 'detecting', detected: 0 })
    const original = new URL(href)
    const mirrors: [number, string, DOMHighResTimeStamp, number][] = []
    await Promise.all(
      mirrorsTemplate.map(async (mirror, index) => {
        const mirrorUrl = mirror.transform(original)
        const result = await checkUrlConnectivity(mirrorUrl)
        if (typeof result === 'number') {
          mirrors.push([index, mirrorUrl, result, -1])
        }
        setLoadState((prev) => {
          if (prev.state === 'detecting') {
            return {
              ...prev,
              detected: prev.detected + 1,
            }
          }
          return prev
        })
      }),
    )
    setLoadState({ state: 'detecting', detected: mirrorsTemplate.length })
    await sleep(300)
    mirrors.sort(([, , a], [, , b]) => a - b)
    try {
      if (Reflect.has(navigator, 'connection')) {
        if (navigator.connection?.saveData) {
          setLoadState({
            state: 'detected',
            availableMirror: mirrors.length,
            canTestSpeed: false,
            cantTestSpeedReason: 'saveData',
          })
        } else if (
          ['slow-2g', '2g', '3g'].includes(
            navigator.connection?.effectiveType || '4g',
          ) ||
          ['bluetooth', 'cellular', 3, 4].includes(
            navigator.connection?.type || 'unknown',
          )
        ) {
          setLoadState({
            state: 'detected',
            availableMirror: mirrors.length,
            canTestSpeed: false,
            cantTestSpeedReason: 'mobile',
          })
        } else {
          throw new Error()
        }
      } else {
        throw new Error()
      }
    } catch {
      for (let [i, [index, mirror]] of mirrors.entries()) {
        setLoadState({
          state: 'speedTesting',
          mirrorIndex: index + 1,
        })
        const mirrorSpeed = await checkUrlSpeed(mirror)
        mirrors[i][3] = mirrorSpeed
      }
      setLoadState({
        state: 'detected',
        availableMirror: mirrors.length,
        canTestSpeed: true,
        cantTestSpeedReason: 'ok',
      })
    }
    mirrors.sort(([, , , a], [, , , b]) => b - a)
    await sleep(500)
    for (const [index, mirror, mirrorLatency, mirrorSpeed] of mirrors) {
      try {
        setLoadState({
          state: 'connecting',
          mirrorIndex: index + 1,
          mirrorLatency,
          mirrorSpeed,
        })
        await sleep(1000)
        await download(mirror, {
          ttfbTimeout: 3500,
          onProgress: (progressEvent) => {
            setLoadState({
              state: 'downloading',
              mirrorIndex: index + 1,
              progressDownloaded: progressEvent.loaded,
              progressTotal: progressEvent.total,
            })
          },
        }).then((blob) => {
          downloadBlob(blob, href.split('/').pop()!)

          setLoadState({ state: 'downloaded' })
        })

        break
      } catch (err) {
        console.warn(
          'download mirror detection: unable to detect download to mirror',
          err,
        )
      }
    }
    setLoadState((prev) => {
      if (prev.state !== 'downloaded') {
        return {
          state: 'fallback',
        }
      }
      return prev
    })
  }, [href])

  useEffect(() => {
    if (loadState.state === 'fallback') {
      console.warn('no mirrors responded correctly; fallback to original URL')
      window.location.href = href
    }
  }, [loadState])

  useEffect(() => {
    if (loadState.state === 'downloading') {
      window.onbeforeunload = () => {
        // this is basically useless lol. all you need is a non-null value to the window.onbeforeunload property
        return 'Please do not close this window until the download is complete.'
      }
    } else {
      window.onbeforeunload = null
    }

    return () => {
      window.onbeforeunload = null
    }
  }, [loadState])

  if (loadState.state === 'idle') {
    return (
      <GlowButton bordered onClick={detectDownload}>
        <div className="flex flex-col items-start whitespace-nowrap">
          <div className="flex items-center -ml-1">
            <Icon icon={platform.platform.icon} fontSize="28px" />
            <span className="ml-2">
              {platform.platform.title}
              <span className="mx-1 text-sm">{platform.platform.subtitle}</span>
              下载
            </span>
          </div>
          <div className="flex items-center mt-1 mb-0.5 ml-8 text-sm">
            <span>{releaseName}</span>
            {platform.asset.download_count && (
              <>
                <Icon icon={mdiDownload} className="ml-2 mr-0.5" />
                <span>
                  {platform.asset.download_count
                    ? platform.asset.download_count.toLocaleString()
                    : '—'}
                </span>
              </>
            )}
          </div>
        </div>
      </GlowButton>
    )
  } else if (loadState.state === 'detecting') {
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title={`正在检测下载镜像可用性 (${loadState.detected}/${mirrorsTemplate.length})……`}
      />
    )
  } else if (loadState.state === 'speedTesting') {
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title={`正在检测下载镜像 #${loadState.mirrorIndex} 速度……`}
      />
    )
  } else if (loadState.state === 'detected') {
    const cantTestSpeedReasonsText = {
      saveData: '用户开启了“节省数据”模式',
      mobile: '用户正在使用移动网络',
      ok: '',
    }
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title={`已检测可用下载镜像 ${loadState.availableMirror} 个（${
          loadState.canTestSpeed
            ? '已按下载速度排序'
            : `由于${
                cantTestSpeedReasonsText[loadState.cantTestSpeedReason]
              }，无法检测镜像下载速度，按镜像延迟排序`
        }）`}
      />
    )
  } else if (loadState.state === 'connecting') {
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title={`正在尝试从镜像 #${
          loadState.mirrorIndex
        }（延迟：${+loadState.mirrorLatency.toFixed(3)} ms${
          loadState.mirrorSpeed > 0
            ? `，测速：${+(
                (loadState.mirrorSpeed / 1024 / 1024) *
                1000
              ).toFixed(3)} MiB/s`
            : ''
        }）下载……`}
      />
    )
  } else if (loadState.state === 'downloading') {
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title={
          <div className="flex items-center">
            <span className="mr-4">
              正在从镜像 #{loadState.mirrorIndex} 下载……
            </span>
            <DataLoadRate
              loaded={loadState.progressDownloaded}
              total={loadState.progressTotal}
            />
          </div>
        }
        className="tabular-nums"
      />
    )
  } else if (loadState.state === 'downloaded') {
    return (
      <DownloadState
        icon={mdiCheck}
        title={platform.platform.messages.downloaded}
      />
    )
  } else if (loadState.state === 'fallback') {
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title="下载失败，正在尝试直接下载……"
      />
    )
  } else {
    return (
      <DownloadState
        icon={mdiAlertCircle}
        title="无效的下载状态，请刷新页面重试"
      />
    )
  }
}

export const DownloadButtons: FC<{ release: Release }> = ({ release }) => {
  const [viewAll, setViewAll] = useState(false)
  const [envPlatformId, setCurrentPlatformId] = useState<
    string | typeof DetectionFailedSymbol | null
  >(null)

  useMount(async () => {
    const platformId = await detectPlatform()
    setCurrentPlatformId(platformId ?? null)
  })

  const validPlatforms = useMemo(
    () =>
      PLATFORMS.reduce((acc, platform) => {
        const asset = platform.assetMatcher(release)
        if (asset)
          acc.push({
            asset,
            platform,
          })
        return acc
      }, [] as ResolvedPlatform[]),
    [release],
  )

  const allPlatformDownloadBtns = useMemo(
    () =>
      validPlatforms.map((platform) => (
        <DownloadButton
          platform={platform}
          releaseName={release.name}
          key={platform.platform.id}
        />
      )),
    [validPlatforms],
  )

  const innerContent = useMemo(() => {
    if (viewAll || envPlatformId === DetectionFailedSymbol) {
      return allPlatformDownloadBtns
    } else {
      const platform = validPlatforms.find(
        (platform) => platform.platform.id === envPlatformId,
      )
      if (!platform) return allPlatformDownloadBtns

      return [
        <DownloadButton
          platform={platform}
          releaseName={release.name}
          key={platform.platform.id}
        />,
      ]
    }
  }, [validPlatforms, viewAll, envPlatformId])

  if (!envPlatformId) {
    return (
      <DownloadState
        iconClassName="animate-spin"
        icon={mdiLoading}
        title="正在匹配……"
      />
    )
  }

  return (
    <AnimatePresence mode="popLayout">
      {validPlatforms.length ? (
        innerContent
      ) : (
        <DownloadState
          key="unavailable"
          icon={mdiAlertCircle}
          title="未找到可用的下载链接"
        />
      )}
      {!viewAll && (
        <div className="flex-row gap-4 items-center flex flex-col md:flex-row">
          <GlowButton
            key="view-all-switch"
            bordered
            onClick={() => setViewAll(true)}
          >
            查看全部
          </GlowButton>
          <div className="text-xs leading-5 text-center text-white/70">
            <a href="https://mirrorchyan.com/zh/download?rid=MAA&os=windows&arch=x64&channel=stable" target="_blank">
              <span><i><u>已有 Mirror酱 CDK？</u></i></span>
              <br></br>
              <span><i><u>前往 Mirror酱 高速下载</u></i></span>
            </a>
          </div>
        </div>
      )}
    </AnimatePresence>
  )
}

export class HomeActionsReleaseErrorBoundary extends Component<{
  children?: React.ReactNode
}> {
  state = {
    error: null as Error | null,
  }

  componentDidCatch(error: Error) {
    this.setState({ error })
  }

  render() {
    const { error } = this.state
    if (error) {
      return (
        <DownloadState
          icon={mdiAlertCircle}
          title={
            <div className="flex flex-col ml-4">
              <span className="mb-2">载入版本信息失败。您可尝试</span>
              <GlowButton
                translucent
                bordered
                href="https://github.com/MaaAssistantArknights/MaaAssistantArknights/releases"
              >
                <span className="text-sm">前往 GitHub Releases 下载</span>
              </GlowButton>
            </div>
          }
        />
      )
    }

    return this.props.children
  }
}

export const HomeActionsRelease: FC = () => {
  const { data } = useRelease()
  return <>{data && <DownloadButtons release={data} />}</>
}
