#include "stdafx.h"
#include "EditEventDlg.h"
#include "util/Util.h"

//----------------------------------------------------------------------------

const EditEventDlg::Actions EditEventDlg::kActionNames[] =
    {
        {L"WM_LBUTTONDOWN", WM_LBUTTONDOWN},
        {L"WM_LBUTTONDBLCLK", WM_LBUTTONDBLCLK},
        {L"WM_RBUTTONDOWN", WM_RBUTTONDOWN},
        {L"WM_RBUTTONDBLCLK", WM_RBUTTONDBLCLK},
        {L"WM_MBUTTONDOWN", WM_MBUTTONDOWN},
        {L"WM_MBUTTONDBLCLK", WM_MBUTTONDBLCLK},
        {L"WM_KEYDOWN", WM_KEYDOWN}};

//----------------------------------------------------------------------------
// Class definition. Public methods

EditEventDlg::EditEventDlg(CString &aWndTitle, CString &aParentWndTitle,
                           CString &aClassName, CString &aParentClassName, CString &aChildrenCount,
                           CString &aActionName, CString &aActionValue, CString &aKeyCode,
                           CString &aDelayTime, bool &aWaitUntilVisible)
    : mWndTitle(aWndTitle), mPWndTitle(aParentWndTitle), mClassName(aClassName),
      mPClassName(aParentClassName), mChildrenCount(aChildrenCount),
      mActionName(aActionName), mActionValue(aActionValue), mKeyCode(aKeyCode),
      mDelayTime(aDelayTime), mWaitUntilVisible(aWaitUntilVisible), mLoaded(false),
      mKeyValueSet(false), mKeyCodeSet(false)
{
}

EditEventDlg::~EditEventDlg()
{
}

LRESULT EditEventDlg::OnInit(HWND /*aWnd*/, LPARAM /*aParam*/)
{
  // center window
  CenterWindow(*this);

  // populate edit controls
  mEditWndTitle.Attach(GetDlgItem(IDC_EDIT_WINDOW_TITLE));
  mEditWndTitle.SetWindowText(mWndTitle);

  mEditPWndTitle.Attach(GetDlgItem(IDC_EDIT_PARENT_WINDOW_TITLE));
  mEditPWndTitle.SetWindowText(mPWndTitle);

  mEditClassName.Attach(GetDlgItem(IDC_EDIT_CLASS_NAME));
  mEditClassName.SetWindowText(mClassName);

  mEditPClassName.Attach(GetDlgItem(IDC_EDIT_PARENT_CLASS_NAME));
  mEditPClassName.SetWindowText(mPClassName);

  mEditChildrenCount.Attach(GetDlgItem(IDC_EDIT_CHILDREN_COUNT));
  mEditChildrenCount.SetWindowText(mChildrenCount);

  mEditActionValue.Attach(GetDlgItem(IDC_EDIT_ACTION_VALUE));
  mEditActionValue.SetWindowText(mActionValue);

  mEditKeyValue.Attach(GetDlgItem(IDC_EDIT_KEY_VALUE));
  mKeyValue += char((_wtoi(mKeyCode)));
  mEditKeyValue.SetWindowText(mKeyValue);

  mEditKeyCode.Attach(GetDlgItem(IDC_EDIT_KEY_CODE));
  mEditKeyCode.SetWindowText(mKeyCode);

  mEditDelayTime.Attach(GetDlgItem(IDC_EDIT_DELAY_TIME));
  mEditDelayTime.SetWindowText(mDelayTime);

  // populate action name combo
  mComboActionName.Attach(GetDlgItem(IDC_COMBO_ACTION_NAME));

  mCheckWaitUntilVisible.Attach(GetDlgItem(IDC_CHECK_WAIT_UNTIL_VISIBLE));
  mCheckWaitUntilVisible.SetCheck(mWaitUntilVisible ? BST_CHECKED
                                                    : BST_UNCHECKED);

  int actionIndex = 0;
  for (int i = 0; i < _countof(kActionNames); i++)
  {
    mComboActionName.InsertString(i, kActionNames[i].mActionName);
    if (kActionNames[i].mActionName == mActionName)
      actionIndex = i;
  }

  mComboActionName.SetCurSel(actionIndex);
  DoComboChanged(IDC_COMBO_ACTION_NAME);

  mLoaded = true;

  DlgResize_Init();
  return 0;
}

LRESULT EditEventDlg::OnClose(UINT /*aNotifyCode*/, WPARAM /*wParam*/,
                              LPARAM /*lParam*/, BOOL & /*aHandled*/)
{
  EndDialog(IDCANCEL);

  return 0;
}

LRESULT EditEventDlg::OnOk(WORD /*aNotifyCode*/, WORD /*aID*/,
                           HWND /*aWndCtl*/, BOOL & /*aHandled*/)
{
  // save changes
  mEditWndTitle.GetWindowText(mWndTitle);
  mEditPWndTitle.GetWindowText(mPWndTitle);
  mEditClassName.GetWindowText(mClassName);
  mEditPClassName.GetWindowText(mPClassName);
  mEditChildrenCount.GetWindowText(mChildrenCount);
  mComboActionName.GetWindowText(mActionName);
  mEditActionValue.GetWindowText(mActionValue);
  mEditKeyCode.GetWindowText(mKeyCode);
  mEditDelayTime.GetWindowText(mDelayTime);
  mWaitUntilVisible = (mCheckWaitUntilVisible.GetCheck() == BST_CHECKED);

  EndDialog(IDOK);

  return 0;
}

LRESULT EditEventDlg::OnCancel(WORD /*aNotifyCode*/, WORD /*aID*/,
                               HWND /*aWndCtl*/, BOOL & /*aHandled*/)
{
  EndDialog(IDCANCEL);

  return 0;
}

LRESULT EditEventDlg::OnComboChanged(WORD /*aNotifyCode*/, WORD aID,
                                     HWND /*aWndCtl*/, BOOL & /*aHandled*/)
{
  DoComboChanged(aID);

  return 0;
}

void EditEventDlg::DoComboChanged(WORD aControlId)
{
  if (aControlId != IDC_COMBO_ACTION_NAME)
    return;

  // get new action name and update action value
  int index = mComboActionName.GetCurSel();
  mActionName = kActionNames[index].mActionName;
  mActionValue =
      StringUtil::LongLongToString(kActionNames[index].mActionValue).c_str();
  mEditActionValue.SetWindowText(mActionValue);
}

LRESULT EditEventDlg::OnChange(WORD /*aNotifyCode*/, WORD aID,
                               HWND /*aWndCtl*/, BOOL & /*aHandled*/)
{
  if (!mLoaded)
    return 0;

  if (aID == IDC_EDIT_KEY_VALUE)
  {
    if (mKeyValueSet)
    {
      mKeyValueSet = false;
      return 0;
    }

    mEditKeyValue.GetWindowText(mKeyValue);

    // allow only 1 character
    if (mKeyValue.GetLength() > 1)
    {
      mKeyValue = mKeyValue[0];
      mKeyValueSet = true;
      mEditKeyValue.SetWindowText(mKeyValue);
    }

    // update key code edit
    wchar_t *keyCode = const_cast<wchar_t *>(mKeyCode.GetString());
    _itow(int(mKeyValue[0]), keyCode, 10);

    mKeyCodeSet = true;
    mEditKeyCode.SetWindowText(keyCode);
  }

  if (aID == IDC_EDIT_KEY_CODE)
  {
    if (mKeyCodeSet)
    {
      mKeyCodeSet = false;
      return 0;
    }

    mEditKeyCode.GetWindowText(mKeyCode);

    // update key value edit
    mKeyValue = char(_wtoi(mKeyCode));

    mKeyValueSet = true;
    mEditKeyValue.SetWindowText(mKeyValue);
  }

  return 0;
}