from pandas import DataFrame
from pathlib import Path

from roguelike.config import Theme

from .main import RecruitPriorityOffset, Oper, Configuration


# contributed by Lancarus
def export_config(output_path: Path, theme: Theme, config: Configuration) -> None:
    filename = f"{theme}_priority.xlsx"
    path = output_path / filename

    table = []

    for group in config.priority:
        for oper_index in range(len(group.opers)):
            oper: Oper = group.opers[oper_index]
            oper_dict = {"group_name": group.name, "priority_in_group": oper_index + 1}
            for oper_field in oper.model_fields:
                if oper_field == "recruit_priority_offsets":
                    offset_list: list[RecruitPriorityOffset] = getattr(oper, oper_field)
                    for offset_index in range(len(offset_list)):
                        offset = offset_list[offset_index]
                        for offset_field in offset.model_fields:
                            oper_dict[f"{offset_field}_{offset_index + 1}"] = getattr(offset, offset_field)
                else:
                    oper_dict[oper_field] = getattr(oper, oper_field)
            table.append(oper_dict)

    # 创建 DataFrame 并将其写入 Excel 文件
    DataFrame(table).to_excel(path, index=False)
