#include "mainWindow.h"

#include <chrono>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlobj_core.h>
#include <algorithm>
#endif // WIN32

using namespace std;

//---------------------------------------------------

const size_t MainWindow::kMaxThemeNameLength = 15;

const stringType MainWindow::kThemeEditorTitle = STRLIT("theme editor");
const stringType MainWindow::kThemePickerTitle = STRLIT("theme picker");
const size_t MainWindow::kThemeWndSpace = 1;
const size_t MainWindow::kOptWndTitleFontSize = 18;
const stringType MainWindow::kBackgroundColorStr = STRLIT("background color (RGB):");
const stringType MainWindow::kTitleColorStr = STRLIT("title color (RGB):");
const stringType MainWindow::kOptionColorStr = STRLIT("option color (RGB):");
const stringType MainWindow::kIconColorStr = STRLIT("icon color (RGB):");
const stringType MainWindow::kSongColorStr = STRLIT("song color (RGB):");
const stringType MainWindow::kThemeNameStr = STRLIT("theme name:");
const size_t MainWindow::kOptionPaddingX = kTitlePaddingX * 2;
const size_t MainWindow::kOptionPaddingY = kTitlePaddingY;
const string MainWindow::kInvalidValue = "invalid value.";
const char MainWindow::kRGBValSepChar = ',';
const stringType MainWindow::kRGBValSep = STRLIT(", ");
const stringType MainWindow::kThemeOptionIdSep = STRLIT(": ");
const stringType MainWindow::kLastThemeName = STRLIT("last theme name: ");

const stringType MainWindow::kDefaultThemeFile = STRLIT("default.sbt");
const stringType MainWindow::kThemeFileExt = STRLIT("sbt");
const stringType MainWindow::kThemeFileDotExt = STRLIT(".sbt");
const stringType MainWindow::kOptionsFileDotExt = STRLIT(".sbo");
const stringType MainWindow::kThemes = STRLIT("themes");
const stringType MainWindow::kOptions = STRLIT("options");
const stringType MainWindow::kFolders = STRLIT("folders");

//---------------------------------------------------

MainWindow::~MainWindow()
{
  mWndThread.join();
}

void MainWindow::ShowWindow()
{
  loadThemes();
  mWndThread = thread(&MainWindow::showWindowThreadProc, this);
}

bool MainWindow::WasClosed()
{
  return mStop;
}

//---------------------------------------------------

bool MainWindow::saveTheme()
{
  if (!mThemes[mCurrentThemeIndex].isCustom)
    return true;

  // open theme options file
  stringType themeFilepath = FolderPaths::makeFilePath(mThemesFolder, mThemes[mCurrentThemeIndex].name, kThemeFileDotExt);
  OFSTREAM(file, themeFilepath);

  if (!file.is_open())
    return false;

  // save theme options
  file << TO_STRING(ThTitleColor) << kThemeOptionIdSep << toString(mThemes[mCurrentThemeIndex].titleColor) << endl;
  file << TO_STRING(ThBackgroundColor) << kThemeOptionIdSep << toString(mThemes[mCurrentThemeIndex].backgroundColor) << endl;
  file << TO_STRING(ThOptionColor) << kThemeOptionIdSep << toString(mThemes[mCurrentThemeIndex].optionColor) << endl;
  file << TO_STRING(ThIconColor) << kThemeOptionIdSep << toString(mThemes[mCurrentThemeIndex].iconColor) << endl;
  file << TO_STRING(ThSongNameColor) << kThemeOptionIdSep << toString(mThemes[mCurrentThemeIndex].songNameColor) << endl;

  mThemes[mCurrentThemeIndex].isCustom = false;
  mThemes[mCurrentThemeIndex].isModified = false;

  file.close();
  return true;
}

vector<stringType> MainWindow::getThemesList()
{
  vector<stringType> themes;
  for (const auto &theme : mThemes)
    themes.push_back(theme.name);

  return themes;
}

void MainWindow::updateThemeEditor()
{
  // load colors
  mThemeEditorCtrls[ThdWndTitle].setFillColor(mThemes[mCurrentThemeIndex].optionsWndTitleColor);
  for (auto &vertex : mThemeEditorTitleLine)
    vertex.color = mThemes[mCurrentThemeIndex].optionsWndTitleColor;

  mThemeEditorCtrls[ThdBackgroundLabel].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdBackgroundEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);
  mThemeEditorCtrls[ThdTitleLabel].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdTitleEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);
  mThemeEditorCtrls[ThdOptionsLabel].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdOptionsEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);
  mThemeEditorCtrls[ThdIconLabel].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdIconEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);
  mThemeEditorCtrls[ThdSongNameLabel].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdSongNameEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);
  mThemeEditorCtrls[ThdNameLabel].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdNameEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);

  mThemeEditorCtrls[ThdRandomButton].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdSaveButton].setFillColor(mThemes[mCurrentThemeIndex].optionColor);
  mThemeEditorCtrls[ThdThemePicker].setFillColor(mThemes[mCurrentThemeIndex].optionColor);

  // load theme strings
  mThemeEditorCtrls[ThdBackgroundEdit].setString(toString(mThemes[mCurrentThemeIndex].backgroundColor));
  mThemeEditorCtrls[ThdTitleEdit].setString(toString(mThemes[mCurrentThemeIndex].titleColor));
  mThemeEditorCtrls[ThdOptionsEdit].setString(toString(mThemes[mCurrentThemeIndex].optionColor));
  mThemeEditorCtrls[ThdIconEdit].setString(toString(mThemes[mCurrentThemeIndex].iconColor));
  mThemeEditorCtrls[ThdSongNameEdit].setString(toString(mThemes[mCurrentThemeIndex].songNameColor));
  mThemeEditorCtrls[ThdNameEdit].setString(mThemes[mCurrentThemeIndex].name);
  updateThemePicker();
}

void MainWindow::handleThemeModification(ThemeComponents iComponent, const stringType &iEditValue)
{
  // if it's not one of the default themes, update it instead of creating a new one
  if (mThemes[mCurrentThemeIndex].isCustom)
  {
    mThemes[mCurrentThemeIndex].isModified = true;

    // update theme name if necessary
    if (iComponent == ThName)
      mThemes[mCurrentThemeIndex].name = iEditValue;
  }
  else
  {
    ThemeInfo customTheme = mThemes[mCurrentThemeIndex];
    customTheme.name = getCustomThemeName(iComponent == ThName ? iEditValue : mThemes[mCurrentThemeIndex].name);
    customTheme.isCustom = true;
    mThemes.push_back(customTheme);
    mCurrentThemeIndex = mThemes.size() - 1;
  }

  // update theme name controls
  mThemeEditorCtrls[ThdNameEdit].setString(mThemes[mCurrentThemeIndex].name);
  mThemeEditorCtrls[ThdNameEdit].setFillColor(mThemes[mCurrentThemeIndex].editableColor);
  updateThemePicker();
}

void MainWindow::handleEditorBackspace(ThemeComponents iComponent, stringType &oEditValue)
{
  if (iComponent == ThBackgroundColor)
    handleEditBackspace(oEditValue, mThemeEditorCtrls[ThdBackgroundEdit]);
  else if (iComponent == ThTitleColor)
    handleEditBackspace(oEditValue, mThemeEditorCtrls[ThdTitleEdit]);
  else if (iComponent == ThOptionColor)
    handleEditBackspace(oEditValue, mThemeEditorCtrls[ThdOptionsEdit]);
  else if (iComponent == ThIconColor)
    handleEditBackspace(oEditValue, mThemeEditorCtrls[ThdIconEdit]);
  else if (iComponent == ThSongNameColor)
    handleEditBackspace(oEditValue, mThemeEditorCtrls[ThdSongNameEdit]);
  else if (iComponent == ThName)
    handleEditBackspace(oEditValue, mThemeEditorCtrls[ThdNameEdit]);
}

void MainWindow::editorGetComponentValue(ThemeComponents iComponent, stringType &oEditValue)
{
  if (iComponent == ThBackgroundColor)
    oEditValue = mThemeEditorCtrls[ThdBackgroundEdit].getString();
  else if (iComponent == ThTitleColor)
    oEditValue = mThemeEditorCtrls[ThdTitleEdit].getString();
  else if (iComponent == ThOptionColor)
    oEditValue = mThemeEditorCtrls[ThdOptionsEdit].getString();
  else if (iComponent == ThIconColor)
    oEditValue = mThemeEditorCtrls[ThdIconEdit].getString();
  else if (iComponent == ThSongNameColor)
    oEditValue = mThemeEditorCtrls[ThdSongNameEdit].getString();
  else if (iComponent == ThName)
    oEditValue = getCustomThemeName(mThemeEditorCtrls[ThdNameEdit].getString());
}

void MainWindow::editorSetComponentInvalid(ThemeComponents iComponent)
{
  if (iComponent == ThBackgroundColor)
    mThemeEditorCtrls[ThdBackgroundEdit].setString(kInvalidValue);
  else if (iComponent == ThTitleColor)
    mThemeEditorCtrls[ThdTitleEdit].setString(kInvalidValue);
  else if (iComponent == ThOptionColor)
    mThemeEditorCtrls[ThdOptionsEdit].setString(kInvalidValue);
  else if (iComponent == ThIconColor)
    mThemeEditorCtrls[ThdIconEdit].setString(kInvalidValue);
  else if (iComponent == ThSongNameColor)
    mThemeEditorCtrls[ThdSongNameEdit].setString(kInvalidValue);
}

bool MainWindow::isValidInput(size_t iNewChar, const stringType &iCurrentValue)
{
  // we're editing RGB values, so allow only this format: "[0..255], [0..255], [0..255]", spaces optional
  if (!isdigit(iNewChar) && (iNewChar != ',') && (iNewChar != ' '))
    return false;

  // start with a digit
  if (iCurrentValue.empty())
    return isdigit(iNewChar);

  // space must be followed by digit
  if (iCurrentValue[iCurrentValue.size() - 1] == ' ')
    return isdigit(iNewChar);

  // comma must be followed by space or digit
  if (iCurrentValue[iCurrentValue.size() - 1] == ',')
    return (iNewChar != ',');

  // max 2 commas and 2 spaces
  if ((iNewChar == ',') && (StringOp::count(iCurrentValue, ',') >= 2))
    return false;

  if ((iNewChar == ' ') && (StringOp::count(iCurrentValue, ' ') >= 2))
    return false;

  // max 3 digits in a row, total value <= 255
  if (isdigit(iNewChar) && (iCurrentValue.size() >= 2) && isdigit(iCurrentValue[iCurrentValue.size() - 1]) && isdigit(iCurrentValue[iCurrentValue.size() - 2]))
  {
    // check for 4th digit after a valid value
    if ((iCurrentValue.size() >= 3) && isdigit(iCurrentValue[iCurrentValue.size() - 3]))
      return false;

    stringType tempString = iCurrentValue.substr(iCurrentValue.size() - 2);
    tempString.push_back(static_cast<charType>(iNewChar));
    if (stoi(tempString) > 255)
      return false;
  }

  return true;
}

bool MainWindow::isValidFilename(size_t iNewChar, const stringType &iCurrentValue)
{
  // only letters and digits for now
  if (!isalnum(iNewChar) && (iNewChar != ' ') && (iNewChar != '_') && (iNewChar != '-') && (iNewChar != '.'))
    return false;

  // must not start with spaces
  if (iCurrentValue.empty() && (iNewChar == ' '))
    return false;

  if (iCurrentValue.size() == kMaxThemeNameLength)
    return false;

  return true;
}

void MainWindow::saveOptions()
{
  // open options file
  stringType optionsFilepath = FolderPaths::makeFilePath(mOptionsFolder, kOptions, kOptionsFileDotExt);
  OFSTREAM(file, optionsFilepath);
  if (!file.is_open())
    return;

  // save options
  file << kLastThemeName << mThemes[mCurrentThemeIndex].name << endl;

  file.close();
}

void MainWindow::loadOptions()
{
  // open options file
  stringType optionsFilepath = mOptionsFolder + kFolderSeparatorStr + kOptions + kOptionsFileDotExt;
  IFSTREAM(file, optionsFilepath);
  if (!file.is_open())
    return;

  // load options
  stringType buffer;
  stringType trimmedBuffer;
  size_t bufferSize = 255;
  buffer.resize(bufferSize);
  while (file.getline(&buffer[0], bufferSize))
  {
    trimmedBuffer = buffer.c_str();
    if (trimmedBuffer.find(kLastThemeName) == 0)
    {
      // set last used theme
      stringType lastThemeName = trimmedBuffer.substr(kLastThemeName.length());
      size_t count = mThemes.size();
      for (size_t i = 0; i < count; i++)
        if (mThemes[i].name == lastThemeName)
        {
          mCurrentThemeIndex = i;
          break;
        }
    }
  }

  file.close();
}
