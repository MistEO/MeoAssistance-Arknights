import json
import os
import sys
import re

# NOTE
# You may customize here
regex_ignore_list = [
    # IGNORE EVENTS
    "\w{2}-Open",
    "\w{2}-OpenOcr",
    "\w{2}ChapterTo\w{2}",
    "DT-8@SideStoryStage",
    "DT-7@SideStoryStage",
    "DT-6@SideStoryStage",
    "DT-3@SideStoryStage",

    # CUSTOM
    "AccountManager",
    "Logout",
    "SSSBuffChoose",
    "Roguelike@LevelName_normal_mode",
    "Reclamation@ClickAnyZoneOcr",
    "MiningActivities",
    "RecruitingActivities",
    "Sami@Roguelike@FoldartalGainOcr",
    "Sami@Roguelike@CheckCollapsalParadigms",
    "Sami@Roguelike@CheckCollapsalParadigmsOnBanner",
    "Sami@Roguelike@CheckCollapsalParadigmsOnPanel",
    "Roguelike@CheckLevelMax",
    "StageDrops-StageCF-FoodBonusFlag",
    "StageDrops-Stage12-TripleFlag",
    "Tales@RA@PIS-ClickTool",

    # FUTURE MODES/EVENTS
    "StartExploreWithSeed"
]

server_list = [
    "YoStarJP",
    "YoStarEN",
    "YoStarKR",
    "txwy"
]

cur_dir = os.path.dirname(os.path.abspath(__file__))
proj_dir = os.path.join(cur_dir, "../../")

def find_missing_translations(server_name):

    output_file = "missing_translate-" + server_name + ".txt"
    output_file = os.path.join(cur_dir, output_file)

    # File name of the json
    # NOTE: You may change this to e.g. recruitment.json
    # This is hardcoded because tasks.json is the most likely modified one
    json_name = "tasks.json"

    zh_json_file = os.path.join(proj_dir, "resource/", json_name)
    gl_json_file = os.path.join(proj_dir, "resource/global/",
                                server_name, "resource/", json_name)

    # For test purpose
    # gl_json_file = os.path.join(cur_dir, "test.json")

    with open(zh_json_file, 'r', encoding='utf-8') as zh_fh:
        zh_json: dict = json.load(zh_fh)

    with open(gl_json_file, 'r', encoding='utf-8') as gl_fh:
        gl_json: dict = json.load(gl_fh)

    # def getBaseTask(task: dict) -> dict:
    #     try:
    #         base_task = zh_json[task["base_task"]]
    #         return getBaseTask(base_task)
    #     except:
    #         return task

    res_keys = []
    for key, value in zh_json.items():
        value: dict
        # check if it has a "text" and if the text is empty
        if value.get("text"):
            # and then check if gl_json has a corresponding one
            try:
                _ = gl_json[key]["text"][0]
            except:
                if any(map(lambda x: re.search(x, key), regex_ignore_list)):
                    continue
                # NOTE
                # It is likely that Ascii texts do not need a translation but there is no guarantee.
                # You may want to comment this out in some cases.
                # if getBaseTask(value).get("isAscii", False) is True:
                if all(map(str.isascii, value["text"])):
                    continue

                res_keys.append(key + ': ' + ', '.join(value["text"]))

                print(key, value["text"])

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(res_keys))

    print("Missing translations written to", output_file)

def main():
    # Get the server name from argument
    # try:
    #     # server_name = "YoStarJP"
    #     server_names = sys.argv[1:]
    #     for server_name in server_names:
    #         if server_name not in server_list:
    #             raise Exception(f"{server_name} is not a valid server name.")
    # except Exception as e:
    #     print(e)
    #     server_names = None
    #     while (not server_names):
    #         print("Enter one or more server names separated by space:",
    #             ", ".join(server_list))
    #         t = input()
    #         server_names = t.split() if all(name in server_list for name in t.split()) else None

    server_names = sys.argv[1:]
    if not server_names:
        server_names = server_list

    for server_name in server_names:
        find_missing_translations(server_name)

if __name__ == "__main__":
    main()
