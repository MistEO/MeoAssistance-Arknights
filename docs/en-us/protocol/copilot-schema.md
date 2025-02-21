---
order: 3
icon: ph:sword-bold
---

# Copilot Schema

Usage of `resource/copilot/*.json` and field description.

## Overview

```json
{
    "stage_name": "暴君",           // Stage name, required. The Chinese name of the level, code, stageId, levelId, etc. are all acceptable as long as they are unique.
    "opers": [                      // Operator list
        {
            "name": "棘刺",         // Operator name

            "skill": 3,             // Skill ID with range [1, 3], optional, 1 by default

            "skill_usage": 0,       // Skill usage, optional, 0 by default
                                    // 0 - Used on `actions`
                                    // 1 - Used when ready (e.g. Thorns: Destreza, Myrtle: Support Beta)
                                    // 2 - Used when ready, only once (e.g. Mountain: Sweeping Stance)
                                    // 3 - Auto-decision (not implemented)
                                    // 0 for auto skills

            "requirements": {       // Requirements, Reserved field, not implemented, optional, empty by default
                "elite": 2,         // Elite requirement, optional, 0 by default
                "level": 90,        // Level requirement, optional, 0 by default
                "skill_level": 10,  // Skill level requirement, optional, 0 by default
                "module": 1,        // Module requirement, optional, 0 by default
                "potentiality": 1   // Potential requirement, optional, 0 by default
            }
        },
    ],
    "groups": [
        {
            "name": "任意正常群奶",  // Group name, required
                                    // Any name is accepted as long as it matches the `name` field in `deploy` below

            "opers": [              // Operators to be chosen randomly, similar to the `opers` field above. Operators with higher levels have higher priority.
                {
                    "name": "夜莺", // Nightingale
                    "skill": 3,
                    "skill_usage": 2 // Skill CD may differ
                },
                {
                    "name": "白面鸮", // Ptilopsis
                    "skill": 2,
                    "skill_usage": 2
                }
            ]
        }
    ],
    "actions": [                    // Actions in order, required. Will be executed sequentially
        {
            "type": "部署",         // Action type, optional, "Deploy" by default
                                    // "Deploy" | "Skill" | "Retreat" | "SpeedUp" | "BulletTime" | "SkillUsage" | "Output" | "SkillDaemon"
                                    // "部署"   |  "技能"  |  "撤退"   | "二倍速"   |  "子弹时间"  |  "技能用法"   | "打印"  |  "摆完挂机"
                                    // Both English and Chinese are supported
                                    // "Deploy" will wait until the cost is enough (unless timeout)
                                    // "Skill" will wait until the skill is ready (unless timeout)
                                    // "SpeedUp" is switchable, i.e. after using it will become 2x speed, and using it again will make it back to normal speed
                                    // "BulletTime", Reserved field, not implemented, it is the 1/5 speed after clicking any operator. Proceeding with any other action will make it back to normal speed
                                    // "Output" will print out the content of the doc (for subtities, etc.)
                                    // "SkillDaemon" will cast skills when they are ready, and do nothing else until the end

            // The relationship between the following three fields is AND, i.e. &&
            "kills": 0,             // Waiting until the number of kills required is reached, optional, 0 by default

            "costs": 50,            // Waiting until the cost for the amount specified, optional, 0 by default
                                    // Note that costs can change due to potential, so this field is only for easy fights. Otherwise, please use "cost_changes"
                                    // Recognition is accurate when the cost is 2 digits. Not recommended for 3-digit costs.

            "cost_changes": 5,      // Waiting until the cost changes for the amount specified, optional, 0 by default
                                    // Note that it calculates since the action starts (based on the cost of the previous action)
                                    // Recognition is accurate when the cost is 2 digits. Not recommended for 3-digit costs.

            "cooling": 2,           // Waiting until the number of operators in CD reaches the number, optional, -1 by default for "ignore".

            // TODO: Other conditions
            // TODO: "condition_type": 0,    // Condition aggregation type, optional, 0 by default
            //                        // 0 - AND； 1 - OR

            "name": "棘刺",         // Operator/group name, required when `type` is "Deploy", Optional when type is "Skill" or "Retreat"

            "location": [ 5, 5 ],   // Position for deployment
                                    // Required when `type` is "Deploy".
                                    // Optional when `type` is "Skill" or "Retreat",
                                    // "Skill": Recommended for site facilities only, using skill with location instead of name, please simply use name to use skill for normal operators
                                    // "Retreat"： Recommended only when multiple summons exist, retreating with location instead of name, please simply use name to retreat for normal operators

            "direction": "左",      // Direction for deployment, required when `type` is "Deploy"
                                    // "Left" | "Right" | "Up" | "Down" | "None"
                                    // "左"   |  "右"   | "上"  | "下"   |  "无"
                                    // Both English and Chinese are supported

            "skill_usage":  1,      // Override skill usage settings, required when `type` is "SkillUsage"
                                    // E.g.: Myrtle needs to attack without using the skill at the beginning, and use the skill automatically later.
                                    // Set it to 1 in that case.

            "pre_delay": 0,         // Pre-delay in ms, optional, 0 by default
                                    // Once all condition parameters of the current action are met, a timer starts, and upon its completion, the action corresponding to the type is executed.
            "post_delay": 0,        // Post-delay in ms, optional, 0 by default
                                    // The current action's type starts a timer upon completion of the action, and once the timer ends, the next action begins.

            // "timeout": 999999999,   // Reserved field, not implemented
                                    // Timeout time in ms, optional when `type` is "Deploy" | "Skill", INT_MAX by default
                                    // Will proceed with the next action when timeout

            "doc": "下棘刺了！",     // Description, optional, displayed on the UI with no effects
            "doc_color": "orange"   // Description colour, optional, displayed on the UI with no effects
        },
                                    // Example 1
        {
            "name": "任意正常群奶",
            "location": [ 5, 6 ],
            "direction": "右"
        },
                                    // Example 2
        {
            "name": "史尔特尔",
            "location": [ 4, 5 ],
            "direction": "左",
            "doc": "你史尔特尔奶奶来啦！",
            "doc_color": "red"
        },
                                    // Example 3
        {
            "type": "二倍速"
        }
    ],
    "minimum_required": "v4.0",     // Minimum required MAA version, required, reserved field, not implemented
    "doc": {                        // Description, optional
        "title": "低练度高成功率作业",
        "title_color": "dark",
        "details": "对练度要求很低balabala……",      // You can write your name here, video link, walkthrough link, etc.
        "details_color": "dark"
    },
    "difficulty": 0 // The difficulty of the task, optional, the default value is 0.
    // 0: Default, not set
    // 1: Support normal difficulty
    // 2: Support raid difficulty
    // 3: Support normal and raid difficulty
}
```

## Example

[OF-1](https://github.com/MaaAssistantArknights/MaaAssistantArknights/blob/master/resource/copilot/OF-1_credit_fight.json)
