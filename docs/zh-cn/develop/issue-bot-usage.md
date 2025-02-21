---
order: 4
icon: bxs:bot
---

# Issue Bot 使用方法

Issue Bot 使用的 action 为 [issue-checker](https://github.com/zzyyyl/issue-checker)，配置文件为 [issue-checker.yml](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/.github/issue-checker.yml)。

::: warning
拉取请求被增加 `ambiguous` 标签是因为没有按照 commitizen 规范提交
:::

## 功能

### 自动触发

- 给 议题 和 拉取请求 增加标签，包括 `module` 系列，`Client` 系列，`ambiguous`，`translation required` 等。  
  Issue Bot 会根据关键词自动增加分类标签。  
  具体关键词可以参考[配置文件](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/.github/issue-checker.yml)。
- 给作者是可见性设置为 public 的 MAA 成员的 议题 和 拉取请求 增加 `MAA Team` 标签。

#### 议题（Issue）及其评论

- 给上传文件失败的议题增加 `incomplete` 标签并评论，告诉用户文件上传失败。

#### 拉取请求（Pull Request）

Issue Bot 会对拉取请求标题的格式进行简单审查。它会增加 `ambiguous` 标签，除非拉取请求标题以下列任一单词开头：

- `build` `chore` `ci` `doc` `docs` `feat` `fix` `perf` `refactor` `rfc` `style` `test`
- `Merge` `merge` `Revert` `revert` `Reapply` `Reapply`

### 手动触发

多使用关键词详细描述问题来自动触发分类标签，少使用下列指令。  
但**当你知道自己的行为会导致 Issue Bot 误解时除外**。

#### 议题（Issue）及拉取请求（Pull Request）

- `Remove {LABEL_NAME}` 可以删除一个标签。
- `Remove labels` 可以删除所有标签。
- `Skip {LABEL_NAME}` 可以跳过一个标签。
- `Skip labels` 可以跳过所有标签。

#### 议题评论（Issue Comments）及拉取请求评论（Pull Request Comments）

- `Skip {LABEL_NAME}` 可以保证不增加标签。
- `Skip labels` 可以保证不增加任何标签。
- 以下几种方法可以为议题增加 `fixed` 标签：<sup>1</sup>  
  - `https://github.com/MaaAssistantArknights/MaaAssistantArknights/commit/{COMMIT_HASH} fixed`
  - `fixed by https://github.com/MaaAssistantArknights/MaaAssistantArknights/commit/{COMMIT_HASH}`
  - `{VERSION} fixed`
  - `fixed by {VERSION}`
- `Duplicate of #{ISSUE_NUMBER}` 可以为当前议题增加 `duplicate` 标签。
- **当你知道自己的议题评论会导致 Issue Bot 误解时，尽量添加一些 skip 操作。**

::: info 注意
<sup>1</sup> 这里的 COMMIT_HASH 需要完整的 40 位
:::

#### 推送（Push）

对于一个推送中的任意提交：

- 在 commit message 中包含以下几种任意一个，可以为对应议题加上 `fixed` 标签：
  - `fix #{ISSUE_NUMBER}`
  - `close #{ISSUE_NUMBER}`
  - `fix https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues/{ISSUE_NUMBER}`
  - `close https://github.com/MaaAssistantArknights/MaaAssistantArknights/issues/{ISSUE_NUMBER}`
