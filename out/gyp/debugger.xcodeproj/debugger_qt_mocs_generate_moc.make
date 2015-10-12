all: \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkCanvasWidget.cpp \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkDebuggerGUI.cpp \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkDrawCommandGeometryWidget.cpp \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkInspectorWidget.cpp \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkSettingsWidget.cpp \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkRasterWidget.cpp \
    $(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkGLWidget.cpp

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkCanvasWidget.cpp \
    : \
    ../debugger/QT/SkCanvasWidget.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkCanvasWidget.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkCanvasWidget.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkCanvasWidget.cpp"

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkDebuggerGUI.cpp \
    : \
    ../debugger/QT/SkDebuggerGUI.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkDebuggerGUI.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkDebuggerGUI.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkDebuggerGUI.cpp"

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkDrawCommandGeometryWidget.cpp \
    : \
    ../debugger/QT/SkDrawCommandGeometryWidget.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkDrawCommandGeometryWidget.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkDrawCommandGeometryWidget.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkDrawCommandGeometryWidget.cpp"

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkInspectorWidget.cpp \
    : \
    ../debugger/QT/SkInspectorWidget.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkInspectorWidget.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkInspectorWidget.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkInspectorWidget.cpp"

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkSettingsWidget.cpp \
    : \
    ../debugger/QT/SkSettingsWidget.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkSettingsWidget.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkSettingsWidget.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkSettingsWidget.cpp"

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkRasterWidget.cpp \
    : \
    ../debugger/QT/SkRasterWidget.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkRasterWidget.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkRasterWidget.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkRasterWidget.cpp"

$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkGLWidget.cpp \
    : \
    ../debugger/QT/SkGLWidget.h
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/debugger/QT"
	@echo note: "Generating SkGLWidget.cpp."
	moc "-DSK_SUPPORT_GPU=1" "../debugger/QT/SkGLWidget.h" -o "$(SHARED_INTERMEDIATE_DIR)/debugger/QT/moc_SkGLWidget.cpp"
