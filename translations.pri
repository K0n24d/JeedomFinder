
#-------------------------------------------------
#
# Gestion des traductions
#  - http://blog.qt.io/blog/2014/03/19/qt-weekly-2-localizing-qt-quick-apps/
#  - https://qt.gitorious.org/qt-labs/weather-app
#
#-------------------------------------------------

# var, prepend, append
defineReplace(prependAll) {
    for(a,$$1):result += $$2$${a}$$3
    return($$result)
}

# Supported languages
LANGUAGES = fr de

# Available translations
TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/Translations/JeedomFinder_, .ts)
QT_TRANSLATIONS_FILES = $$prependAll(LANGUAGES, qt_, .qm)

# Used to embed the qm files in resources
TRANSLATIONS_FILES =

# run LRELEASE to generate the qm files
qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$OUT_PWD/Translations/$$basename(tsfile)
#    qmfile = $$shadowed($$tsfile)
    qmfile ~= s,\\.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        command = mkdir -p $$qmdir
        system($$command)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm  $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES += $$qmfile
}

# Create the resource file
GENERATED_RESOURCE_FILE = $$OUT_PWD/translations.qrc

RESOURCE_CONTENT = \
    "<RCC>" \
    "<qresource>"

for(translationfile, TRANSLATIONS_FILES) {
#    relativepath_out = $$relative_path($$translationfile, $$OUT_PWD)
    relativepath_out = $$translationfile
    relativepath_out ~= s,$$OUT_PWD/,,
    RESOURCE_CONTENT += "<file alias=\\\"$$relativepath_out\\\">$$relativepath_out</file>"
}

static:for(translationfile, QT_TRANSLATIONS_FILES) {
#    relativepath_out = $$relative_path($$translationfile, $$OUT_PWD)
#    RESOURCE_CONTENT += "<file alias=\"$$relativepath_out\">$$relativepath_out</file>"
    RESOURCE_CONTENT += "<file alias=\\\"Translations/$$translationfile\\\">$$[QT_INSTALL_TRANSLATIONS]/$$translationfile</file>"
}

RESOURCE_CONTENT += \
    "</qresource>" \
    "</RCC>"

command = echo \"$$RESOURCE_CONTENT\" > $$GENERATED_RESOURCE_FILE
system($$command)|error("Aborting.")

RESOURCES += $$GENERATED_RESOURCE_FILE

# TRANSLATIONS - Create extra targets for convenience

wd = $$replace(PWD, /, $$QMAKE_DIR_SEP)

# LUPDATE - Make new targets for each and all languages
qtPrepareTool(LUPDATE, lupdate)
LUPDATE += -locations relative -no-ui-lines
TSFILES = $$files($$PWD/Translations/JeedomFinder_*.ts) $$PWD/Translations/JeedomFinder_untranslated.ts
for(file, TSFILES) {
    lang = $$replace(file, .*_([^/]*)\\.ts, \\1)
    v = ts-$${lang}.commands
    $$v = cd $$wd && $$LUPDATE $$SOURCES $$APP_FILES -ts $$file
    QMAKE_EXTRA_TARGETS += ts-$$lang
}
ts-all.commands = cd $$PWD && $$LUPDATE $$SOURCES $$APP_FILES -ts $$TSFILES
QMAKE_EXTRA_TARGETS += ts-all

# COMMIT - Make a new target for lconvert and committing the ts files
# lconvert is used to remove the strings location in the ts files
# and thus save space.
qtPrepareTool(LCONVERT, lconvert)
LCONVERT += -locations none
isEqual(QMAKE_DIR_SEP, /) {
    commit-ts.commands = \
        cd $$wd; \
        git add -N Translations/*_??.ts && \
        for f in `git diff-files --name-only Translations/*_??.ts`; do \
            $$LCONVERT -i \$\$f -o \$\$f; \
        done; \
        git add translations/*_??.ts && git commit
} else {
    commit-ts.commands = \
        cd $$wd && \
        git add -N Translations/*_??.ts && \
        for /f usebackq %%f in (`git diff-files --name-only -- Translations/*_??.ts`) do \
            $$LCONVERT -i %%f -o %%f $$escape_expand(\\n\\t) \
        cd $$wd && git add Translations/*_??.ts && git commit
}
QMAKE_EXTRA_TARGETS += commit-ts

