from os import system
from pathlib import Path

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QCheckBox, QComboBox, QDesktopWidget, QFileDialog, QHBoxLayout, QLabel, QLineEdit,
                             QMainWindow, QMessageBox, QPlainTextEdit, QPushButton, QSizePolicy, QStyleFactory,
                             QTabWidget, QVBoxLayout, QWidget)

from pydantic import ValidationError

from roguelike.config import Theme
from roguelike.recruitment import Configuration, export_config

from .models import VisualisationModel
from .views import GroupListView, OffsetATableView, OffsetBTableView, OperListView, TabBar, TableView


class MainWindow(QMainWindow):
    def __init__(self, configurations: dict[Theme, Configuration],
                 parent: QWidget = None,
                 flags: Qt.WindowFlags = Qt.WindowFlags()):
        super().__init__(parent, flags)

        self.configurations: list[Configuration] = configurations

        self.set_style: Callable[str, None] = lambda: None

        self.screen = QDesktopWidget().screenGeometry()
        self.setFixedSize(self.screen.width() * 3 // 5, self.screen.height() * 3 // 4)

        self.setWindowTitle(f"MAA Roguelike Recruitment Management Tool")
        self.version_label = QLabel(f"Version:")
        self.statusBar().addPermanentWidget(self.version_label)

        self.central_widget = QWidget(self)
        self.central_layout = QVBoxLayout(self.central_widget)
        self.setCentralWidget(self.central_widget)

        # ======== main_widget ===========================================
        self.main_widget = QWidget(self.central_widget)
        self.main_layout = QHBoxLayout(self.main_widget)
        self.central_layout.addWidget(self.main_widget)

        # ———————— display_widget ————————————————————————————————————————
        self.display_widget = QWidget(self.main_widget)
        self.display_layout = QVBoxLayout(self.display_widget)
        self.main_layout.addWidget(self.display_widget)

        # -------- visualisation_widget ----------------------------------
        self.visualisation_widget = QWidget(self.display_widget)
        self.visualisation_layout = QHBoxLayout(self.visualisation_widget)
        self.visualisation_layout.setContentsMargins(0, 0, 0, 0)
        self.display_layout.addWidget(self.visualisation_widget)

        # visualisation_model
        self.visualisation_model = VisualisationModel(self.visualisation_widget)

        # group_list_view
        tmp_widget = QWidget(self.visualisation_widget)
        tmp_layout = QVBoxLayout(tmp_widget)
        tmp_layout.setAlignment(Qt.AlignCenter)
        tmp_layout.setContentsMargins(0, 0, 0, 0)
        tmp_layout.setSpacing(0)
        self.visualisation_layout.addWidget(tmp_widget)

        tmp_label = QLabel("Group List")
        tmp_label.setAlignment(Qt.AlignCenter)
        tmp_label.setFixedHeight(self.height() // 32)
        tmp_layout.addWidget(tmp_label)

        self.group_list_view = GroupListView(self.visualisation_widget)
        self.group_list_view.setModel(self.visualisation_model.group_list_model)
        self.group_list_view.selectionModel().selectionChanged.connect(self.visualisation_model.on_group_selection)
        self.group_list_view.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        tmp_layout.addWidget(self.group_list_view)

        self.visualisation_layout.addStretch()

        # oper_list_view
        tmp_widget = QWidget(self.visualisation_widget)
        tmp_layout = QVBoxLayout(tmp_widget)
        tmp_layout.setAlignment(Qt.AlignCenter)
        tmp_layout.setContentsMargins(0, 0, 0, 0)
        tmp_layout.setSpacing(0)
        self.visualisation_layout.addWidget(tmp_widget)

        tmp_label = QLabel("Oper List")
        tmp_label.setAlignment(Qt.AlignCenter)
        tmp_label.setFixedHeight(self.height() // 32)
        tmp_layout.addWidget(tmp_label)

        self.oper_list_view = OperListView(self.visualisation_widget)
        self.oper_list_view.setModel(self.visualisation_model.oper_list_model)
        self.oper_list_view.selectionModel().selectionChanged.connect(self.visualisation_model.on_oper_selection)
        self.oper_list_view.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        tmp_layout.addWidget(self.oper_list_view)

        self.visualisation_layout.addStretch()

        # oper_details_widget
        tmp_widget = QWidget(self.visualisation_widget)
        tmp_layout = QVBoxLayout(tmp_widget)
        tmp_layout.setContentsMargins(0, 0, 0, 0)
        self.visualisation_layout.addWidget(tmp_widget)

        self.oper_details_widget = QTabWidget(self.visualisation_widget)
        self.oper_details_widget.setFixedWidth(self.width() // 2)
        self.oper_details_widget.setTabBar(TabBar(self))
        self.oper_details_widget.tabBar().setFixedHeight(self.height() // 32)
        self.oper_details_widget.setDocumentMode(True)
        tmp_layout.addWidget(self.oper_details_widget)

        # oper_info_view
        self.oper_info_widget = TableView(self.oper_details_widget)
        self.oper_info_widget.setModel(self.visualisation_model.oper_info_model)
        self.oper_details_widget.addTab(self.oper_info_widget, "Info")

        # offset_A_widget
        self.offset_A_widget = OffsetATableView(self.oper_details_widget)
        self.offset_A_widget.setModel(self.visualisation_model.oper_offset_A_model)
        self.oper_details_widget.addTab(self.offset_A_widget, "Offset A")

        # offset_B_widget
        self.offset_B_widget = OffsetBTableView(self.oper_details_widget)
        self.offset_B_widget.setModel(self.visualisation_model.oper_offset_B_model)
        self.oper_details_widget.addTab(self.offset_B_widget, "Offset B")

        self.oper_details_widget.setCurrentIndex(0)

        # -------- log_widget --------------------------------------------
        self.log_widget = QPlainTextEdit(self.display_widget)
        self.log_widget.setReadOnly(True)
        self.log_widget.setFixedHeight(self.height() // 10)
        self.display_layout.addWidget(self.log_widget)

        # ———————— control_widget ———————————————————————————————————
        self.control_widget = QWidget(self.main_widget)
        self.control_layout = QVBoxLayout(self.control_widget)
        self.control_layout.setAlignment(Qt.AlignCenter)
        self.control_widget.setFixedWidth(self.width() // 8)
        self.main_layout.addWidget(self.control_widget)

        # -------- style_combo_box ---------------------------------------
        self.style_combo_box = QComboBox(self.control_widget)
        for style_name in QStyleFactory.keys():
            self.style_combo_box.addItem(style_name, style_name)
        self.style_combo_box.setCurrentIndex(QStyleFactory.keys().index(self.style().objectName()))
        self.style_combo_box.currentIndexChanged.connect(lambda: self.set_style(self.style_combo_box.currentData()))
        self.control_layout.addWidget(self.style_combo_box)

        # -------- theme_combo_box ---------------------------------------
        self.theme_combo_box = QComboBox(self.control_widget)
        self.theme_combo_box.setEnabled(False)
        for theme in Theme:
            self.theme_combo_box.addItem(theme, theme)
        self.theme_combo_box.setCurrentIndex(self.theme_combo_box.count() - 1)
        self.theme_combo_box.currentIndexChanged.connect(
            lambda: self.visualisation_model.set_configuration(self.configurations[self.theme_combo_box.currentData()])
        )
        self.control_layout.addWidget(self.theme_combo_box)

        # -------- search_line_widget ---------------------------------------
        self.search_line_widget = QLineEdit()
        self.control_layout.addWidget(self.search_line_widget)

        # -------- find_next_button ---------------------------------------
        self.find_next_button = QPushButton("Find Next")
        self.find_next_button.setEnabled(False)
        self.find_next_button.setFixedWidth(100)
        self.find_next_button.clicked.connect(self.on_find_next)
        self.control_layout.addWidget(self.find_next_button)

        # -------- validate_button ---------------------------------------
        self.validate_button = QPushButton("Validate")
        self.validate_button.setEnabled(False)
        self.validate_button.setFixedWidth(100)
        self.validate_button.clicked.connect(self.on_validate)
        self.control_layout.addWidget(self.validate_button)

        # -------- save_button -----------------------------------------
        self.save_button = QPushButton("Save")
        self.save_button.setEnabled(False)
        self.save_button.setFixedWidth(100)
        self.save_button.clicked.connect(self.on_save)
        self.control_layout.addWidget(self.save_button)

        self.prettier_checkbox_widget = QCheckBox("Prettier")
        self.prettier_checkbox_widget.setChecked(True)
        self.control_layout.addWidget(self.prettier_checkbox_widget)

        # -------- export_button -----------------------------------------
        self.export_button = QPushButton("Export")
        self.export_button.setEnabled(False)
        self.export_button.setFixedWidth(100)
        self.export_button.clicked.connect(self.on_export)
        self.control_layout.addWidget(self.export_button)

        # ======== resource_dir_widget ===================================
        self.resource_dir_widget = QWidget(self.main_widget)
        self.resource_dir_layout = QHBoxLayout(self.resource_dir_widget)
        self.central_layout.addWidget(self.resource_dir_widget)

        self.resource_dir_layout.addWidget(QLabel("Resource Directory"))

        self.resource_dir_line_widget = QLineEdit()
        self.resource_dir_layout.addWidget(self.resource_dir_line_widget)

        # ———————— browse_button —————————————————————————————————————————
        self.browse_button = QPushButton("Browse")
        self.browse_button.setFixedWidth(100)
        self.browse_button.clicked.connect(self.on_browse)
        self.resource_dir_layout.addWidget(self.browse_button)

        # ———————— load_button ———————————————————————————————————————————
        self.load_button = QPushButton("Load")
        self.load_button.setFixedWidth(100)
        self.load_button.clicked.connect(self.on_load)
        self.resource_dir_layout.addWidget(self.load_button)

    def process_exception(self, e: Exception, heading: str = "", title: str | None = None):
        if title is None:
            title = heading
        if isinstance(e, ValidationError):
            msg = "\n".join(error["msg"] for error in e.errors())
        else:
            msg = str(e)
        self.log_widget.appendPlainText(msg)
        QMessageBox.critical(self, title, f"{heading}\n\n{msg}")

    # ================================================================
    # resource_dir
    # ================================================================

    def on_browse(self):
        file_dialog = QFileDialog(self.resource_dir_line_widget)
        file_dialog.setFileMode(QFileDialog.Directory)
        file_dialog.setOption(QFileDialog.ShowDirsOnly, True)

        file_dialog.setDirectory(self.resource_dir_line_widget.text())
        if file_dialog.exec_():
            selected_directory = file_dialog.selectedFiles()[0]
            if selected_directory:
                self.resource_dir_line_widget.setText(selected_directory)

    def on_load(self):
        for theme in Theme:
            self.log_widget.appendPlainText(f"Loading {theme} ...")
            try:
                config_path = Path(
                    self.resource_dir_line_widget.text()) / "roguelike" / theme / "recruitment.json"
                with open(config_path, encoding="utf-8") as fp:
                    self.configurations[theme] = Configuration.json2config(fp.read())
            except Exception as e:
                self.set_control_enabled(False)

                self.configurations.clear()
                self.visualisation_model.set_configuration(None)

                self.log_widget.insertPlainText(f"Failed!")
                self.process_exception(e, f"Error when loading {theme}")
                break
            else:
                self.log_widget.insertPlainText("Succeeded!")
        else:
            self.visualisation_model.set_configuration(self.configurations[self.theme_combo_box.currentData()])
            self.set_control_enabled(True)
            self.log_widget.appendPlainText(f"Done!")

    # ================================================================
    # control
    # ================================================================

    def set_control_enabled(self, enabled: bool):
        self.theme_combo_box.setEnabled(enabled)
        self.find_next_button.setEnabled(enabled)
        self.validate_button.setEnabled(enabled)
        self.save_button.setEnabled(enabled)
        self.export_button.setEnabled(enabled)

    def on_find_next(self):
        target_oper_name = self.search_line_widget.text()
        selected_configuration = self.visualisation_model.get_configuration()
        if selected_configuration is None:
            return
        selected_group_index = self.visualisation_model.get_selected_group_index()
        selected_oper_index = self.visualisation_model.get_selected_oper_index()
        group_index_begin: int = selected_group_index if selected_group_index is not None else 0
        for group_index in range(group_index_begin, len(selected_configuration.priority)):
            tmp_selected_group: Group = selected_configuration.priority[group_index]
            oper_index_begin: int = selected_oper_index + 1 \
                if group_index == selected_group_index and selected_oper_index is not None \
                else 0
            for oper_index in range(oper_index_begin, len(tmp_selected_group.opers)):
                tmp_selected_oper = tmp_selected_group.opers[oper_index]
                if tmp_selected_oper.name == target_oper_name:
                    group_model_index = self.visualisation_model.group_list_model.index(group_index)
                    self.group_list_view.selectionModel().clearSelection()
                    self.group_list_view.selectionModel().select(
                        group_model_index, self.group_list_view.selectionModel().Select)
                    self.group_list_view.scrollTo(group_model_index, self.group_list_view.PositionAtCenter)

                    oper_model_index = self.visualisation_model.oper_list_model.index(oper_index)
                    self.oper_list_view.selectionModel().clearSelection()
                    self.oper_list_view.selectionModel().select(
                        oper_model_index, self.oper_list_view.selectionModel().Select)
                    self.oper_list_view.scrollTo(oper_model_index, self.oper_list_view.PositionAtCenter)
                    break
            else:
                continue
            break

    def on_validate(self):
        for theme in Theme:
            self.log_widget.appendPlainText(f"Validating {theme} ...")
            try:
                Configuration.model_validate(self.configurations[theme])
            except ValidationError as e:
                self.log_widget.insertPlainText(f"Failed!")
                self.process_exception(e, f"Error when validating {theme}")
                break
            else:
                self.log_widget.insertPlainText("Passed!")
        else:
            self.log_widget.appendPlainText(f"Done!")

    def on_save(self):
        for theme in Theme:
            self.log_widget.appendPlainText(f"Saving {theme} ...")
            try:
                config_path = Path(
                    self.resource_dir_line_widget.text()) / "roguelike" / theme / "recruitment.json"
                json_str = Configuration.config2json(self.configurations[theme])
                with open(config_path, "w", encoding="utf-8") as fp:
                    fp.write(json_str)
                if self.prettier_checkbox_widget.isChecked():
                    system("prettier -w " + str(config_path))
            except Exception as e:
                self.log_widget.insertPlainText(f"Failed!")
                self.process_exception(e, f"Error when saving {theme}")
                break
            else:
                self.log_widget.insertPlainText("Succeeded!")
        else:
            self.log_widget.appendPlainText(f"Done!")

    def on_export(self):
        output_path = Path(__file__).resolve().parent.parent / "output"
        output_path.mkdir(parents=True, exist_ok=True)
        for theme in Theme:
            self.log_widget.appendPlainText(f"Exporting {theme} ...")
            try:
                export_config(output_path, theme, self.configurations[theme])
            except Exception as e:
                self.log_widget.insertPlainText(f"Failed!")
                self.process_exception(e, f"Error when exporting {theme}")
                break
            else:
                self.log_widget.insertPlainText("Succeeded!")
        else:
            self.log_widget.appendPlainText(f"Done!")
