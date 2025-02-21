---
order: 5
icon: ri:earth-fill
---

# 外服适配教程

## 准备工作

在开始这个教程之前，请确保你已经：

1. 安装并正确配置了所需软件。在国服或相应客户端的 `readme.md` 中应该会有相关信息，确保所支持的功能可以正常运行。
2. 阅读了 [任务流程协议](../protocol/task-schema.md)，对各个字段的含义和用法有基本了解，并能理解 `@`、`#` 类型任务的含义和用法。
3. 了解外服的 `task.json` 和模板图片中未提及的和缺少的内容会使用国服的 `task.json` 和模板图片等内容作为备选。外服的 `task.json` 中的内容会覆盖并重写国服对应任务的相应字段。
4. 具备一定的英语能力，能够阅读英文日志，并能通过日志找出缺失的图片等信息。
5. 建议按照任务链进行修改。例如，对于 `Award` 任务，根据国服 `task.json` 中 `Award` 任务的 `next` 顺序逐步替换 `模板图片` / `文本` / `修改 roi`，以确保修改后的每一步都能正常运行，或者能够迅速发现错误。这样可以避免因为一次修改修改了太多内容而不清楚程序因为哪一步卡住而不能运行。

### 修改前准备

在进行修改之前，有几个准备工作需要注意：

1. 参考国服 task.json，确保你已经准备好了与国服不同的用于外服的模板图片和文本内容。
2. 确保你能够随时获取这些图片和文本内容。

## 获取截图

为了获取高质量的截图，请遵循以下指南：

1. 使用模拟器自带的截图工具进行截图并保存。
2. 确保截图的尺寸大于 `1280*720`，长宽比为 `16:9`。
3. 确保截图中不包含任何无关内容，例如任务栏、状态栏、通知栏等。
4. 确保截图中包含所有需要识别的内容。

为了裁剪图片并获取文本/图片 `roi`，你需要安装 `python` 和 `opencv`，并下载 `MaaAssistantArknights/tools/CropRoi/main.py` 文件。

然后，按照以下步骤操作：

1. 在 `main.py` 同目录下新建 `src` 和 `dst` 文件夹。
2. 将需要修改大小或获取文本/图片 `roi` 的 **完整截图** 放入 `src` 文件夹。
3. 运行 `main.py`。
4. 使用鼠标拖动选择目标范围，尽量不要包含无关内容。
5. 确定范围后，按 `S` 键保存，按 `Q` 键退出。裁剪后的图像将被保存在 `dst` 文件夹。

例如完成一次裁剪后的输出内容为：

```log
src: Screenshot_xxx.png
dst: Screenshot_xxx.png_426,272,177,201.png
original roi: 476, 322, 77, 101,
amplified roi: 426, 272, 177, 201
```

其中，

`Screenshot_xxx.png` 为放入 `src` 文件夹的完整截图的名称。`Screenshot_xxx.png_426,272,177,201.png` 为截取后的图片。

`original roi` 为鼠标选取的区域。`amplified roi` 为扩大后的区域，你需要的是扩大后的范围，因此在 `task.json` 中的 `roi` 字段填入的就是这个值。

## 修改模板图片

在修改模板图片之前，需要打开对应客户端的模板图片文件夹和国服的模板图片文件夹。

例如：

- 美服的模板图片文件夹位置为 `MaaAssistantArknights\resource\global\YoStarEN\resource\template`。
- 国服的模板图片文件夹位置为 `MaaAssistantArknights\resource\template`。

参考 `task.json` 中提到的模板图片，对比国服和外服的模板图片，找出外服中缺少的模板。

通常情况下，除了标志等图片，包含文字的模板都需要通过截图来替换。如果图片尺寸明显大于国服对应模板图片，则需要修改 `roi` 的大小。

将截取并重命名完成的模板图片放入对应客户端的模板图片文件夹。

## 修改文本内容

在修改文本内容之前，需要打开对应服务器的 `task.json` 和国服的 `task.json`。

例如：

- 美服的 `task.json` 位置为 `MaaAssistantArknights\resource\global\YoStarEN\resource\tasks.json`。
- 国服的 `task.json` 位置为 `MaaAssistantArknights\resource\tasks.json`。

找到对应任务，将 `text` 字段修改为对应服务器内显示的内容。注意，识别的内容可以是游戏内完整内容的子串。

通常情况下，除非是纯 ASCII 字符识别，否则包含文字的 `text` 都需要替换。如果文字长度明显大于国服，则需要修改 `roi` 的大小，如 `"任务"` 和 `"Mission"` 长度差距过大，则需要修改外服该任务 `roi` 的大小。

如果对应外服的 `task.json` 中没有该任务，则需要添加任务，只需要填写 `text` 字段即可。

## 修改 ROI 范围

1. 打开对应服务器的 `task.json`，如美服的位置为 `MaaAssistantArknights\resource\global\YoStarEN\resource\tasks.json`
2. 找到对应需要修改的 `roi` 范围的任务，使用您准备好的外服游戏界面截图，根据 `amplified roi`，调整 `roi` 范围的大小。
3. 通常情况下， `roi` 不需要修改，只有和国服的识别内容大小差距过大时才需要修改。
4. 如对应外服的 `task.json` 中任务不存在，则添加任务，写上 `roi` 字段。

## 保存设置并重新启动软件

在修改完成后，重新启动软件，重新加载文件，使修改生效。或在软件目录下新建一个 `DEBUG.txt` 后再打开软件，这样每次点击开始都会重新加载模板和文件，不需要重启。

检查是否成功：

1. 检查软件的运行情况，确保软件能够在外服中正常使用。
2. 如不能正常运行，需要检查修改是否正确，或查看日志输出，从而找到出错的地方。

## 解读日志

有些时候，我们修改完了 `task.json` 之后发现程序仍然不能正确运行，这时候我们考虑考虑查看日志找到出错的地方，从而修改对应任务。

日志文件的位置在软件的根目录下，文件名为 `asst.log`。如果你是自己编译的 MAA ，则在 `\x64\Release` 或 `x64\RelWithDebInfo` ，具体在哪个文件夹取决你编译时选择的编译模式。

下面是一段日志示例：

```log
[2022-12-18 17:43:17.535][INF][Px7ec][Tx15c8] {"taskchain":"Award","details":{"to_be_recognized":["Award@ReturnTo","Award","ReceiveAward","DailyTask","WeeklyTask","Award@CloseAnno","Award@CloseAnnoTexas","Award@TodaysSupplies","Award@FromStageSN"],"cur_retry":10,"retry_times":20},"first":["AwardBegin"],"taskid":2,"class":"asst::ProcessTask","subtask":"ProcessTask","pre_task":"AwardBegin"}
[2022-12-18 17:43:18.398][INF][Px7ec][Tx15c8] Call ` C:\Program Files\BlueStacks_nxt\.\HD-Adb.exe -s 127.0.0.1:5555 exec-out "screencap | gzip -1" ` ret 0 , cost 862 ms , stdout size: 2074904 , socket size: 0
[2022-12-18 17:43:18.541][TRC][Px7ec][Tx15c8] OcrPack::recognize | roi: [ 500, 50, 300, 150 ]
[2022-12-18 17:43:18.541][TRC][Px7ec][Tx15c8] Ocr Pipeline with asst::WordOcr | enter
[2022-12-18 17:43:18.634][TRC][Px7ec][Tx15c8] Ocr Pipeline with asst::WordOcr | leave, 93 ms
[2022-12-18 17:43:18.634][TRC][Px7ec][Tx15c8] OcrPack::recognize | raw: [{ : [ 0, 0, 300, 150 ], score: 0.000000 }]
[2022-12-18 17:43:18.634][TRC][Px7ec][Tx15c8] OcrPack::recognize | proc: []
[2022-12-18 17:43:18.637][TRC][Px7ec][Tx15c8] asst::ProcessTask::_run | leave, 1101 ms
[2022-12-18 17:43:18.638][TRC][Px7ec][Tx15c8] ready to sleep 500
[2022-12-18 17:43:19.144][TRC][Px7ec][Tx15c8] end of sleep 500
[2022-12-18 17:43:19.144][TRC][Px7ec][Tx15c8] asst::ProcessTask::_run | enter
```

在这段日志中，你可以看到：

- `taskchain` 代表当前进行中的任务。
- `details` 是任务的内容，包括需要识别的字段（`to_be_recognized`）和当前重试次数（`cur_retry`）和总重试次数（`retry_times`）。
- `first` 代表任务的开始。
- `taskid` 是任务的编号。
- `class` 和 `subtask` 分别代表任务的类别和子任务。
- `pre_task` 代表前一个任务。
  此外，日志中还会记录命令的执行情况（如 `Call`）和 `OCR` 的信息（如 `OcrPack::recognize`）。

在这段日志中`"to_be_recognized"`,`"cur_retry":3,"retry_times":20` 表示已经重复识别了 3 次，最大识别次数为 20 次，到了最大识别次数后会跳过该任务并报错，继续执行下一个任务。如果前面的任务没有问题，我们基本可以确定是这里的识别出了问题，需要查看日志中的提到的任务，寻找是否有对应的 `模板文件`，对应任务的 `text` 是否错误，任务识别 `roi` 范围是否正确，从而找出问题所在并修改。

通过查看对应模板图片，发现美服模板文件夹中有这张图的模板，但是大小明显大于国服图片，导致国服的 `roi` 用在美服上识别不出来，所以需要修改美服的 `task.json` 中的对应任务 `roi`，使其与美服图片大小对应。

## 提交你的修改

请参考 [Github Pull Request 指南](./pr-tutorial.md)
