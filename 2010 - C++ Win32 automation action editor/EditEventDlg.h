#ifndef _EDIT_EVENT_DLG_H_
#define _EDIT_EVENT_DLG_H_

#include "resources/resource.h"

//----------------------------------------------------------------------------
// Class declaration.

/**
 * Class implementing the Edit Event dialog box.
 */
class EditEventDlg : public CDialogImpl<EditEventDlg>,
                     public CDialogResize<EditEventDlg>
{
public:
  enum
  {
    IDD = IDD_EDIT_EVENT
  };

  BEGIN_MSG_MAP_EX(EditEventDlg)
  MSG_WM_INITDIALOG(OnInit)
  MESSAGE_HANDLER(WM_CLOSE, OnClose)
  COMMAND_ID_HANDLER(IDOK, OnOk)
  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
  COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnComboChanged)
  COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
  CHAIN_MSG_MAP(CDialogResize<EditEventDlg>)
  REFLECT_NOTIFICATIONS()
  END_MSG_MAP()

  BEGIN_DLGRESIZE_MAP(EditEventDlg)
  DLGRESIZE_CONTROL(IDC_EVENT_DETAILS_GRP, DLSZ_SIZE_X | DLSZ_SIZE_Y)
  DLGRESIZE_CONTROL(IDC_EDIT_WINDOW_TITLE, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_PARENT_WINDOW_TITLE, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_CLASS_NAME, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_PARENT_CLASS_NAME, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_CHILDREN_COUNT, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_COMBO_ACTION_NAME, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_ACTION_VALUE, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_KEY_CODE, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_EDIT_DELAY_TIME, DLSZ_SIZE_X)
  DLGRESIZE_CONTROL(IDC_ST_DELAY_TIME_MS, DLSZ_MOVE_X)
  DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
  DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
  END_DLGRESIZE_MAP()

  EditEventDlg(CString &aWndTitle, CString &aParentWndTitle,
               CString &aClassName, CString &aParentClassName, CString &aChildrenCount,
               CString &aActionName, CString &aActionValue, CString &aKeyCode,
               CString &aDelayTime, bool &aWaitUntilVisible);
  ~EditEventDlg();

private:
  typedef struct
  {
    CString mActionName;
    UINT mActionValue;
  } Actions;

  static const Actions kActionNames[];

  CEdit mEditWndTitle;
  CEdit mEditPWndTitle;
  CEdit mEditClassName;
  CEdit mEditPClassName;
  CEdit mEditChildrenCount;
  CEdit mEditActionValue;
  CEdit mEditKeyValue;
  CEdit mEditKeyCode;
  CEdit mEditDelayTime;
  CComboBox mComboActionName;
  CButton mCheckWaitUntilVisible;

  CString &mWndTitle;
  CString &mPWndTitle;
  CString &mClassName;
  CString &mPClassName;
  CString &mChildrenCount;
  CString &mActionName;
  CString &mActionValue;
  CString mKeyValue;
  CString &mKeyCode;
  CString &mDelayTime;
  bool &mWaitUntilVisible;

  bool mLoaded;
  bool mKeyValueSet;
  bool mKeyCodeSet;

  // standard event handlers
  LRESULT OnInit(HWND aWnd, LPARAM aParam);
  LRESULT OnClose(UINT aNotifyCode, WPARAM wParam, LPARAM lParam,
                  BOOL &aHandled);
  LRESULT OnOk(WORD aNotifyCode, WORD aID, HWND aWndCtl,
               BOOL &aHandled);
  LRESULT OnCancel(WORD aNotifyCode, WORD aID, HWND aWndCtl,
                   BOOL &aHandled);

  // handle action name combo
  LRESULT OnComboChanged(WORD aNotifyCode, WORD aID, HWND aWndCtl,
                         BOOL &aHandled);
  void DoComboChanged(WORD aControlId);

  // handle a change in the key value edit
  LRESULT OnChange(WORD aNotifyCode, WORD aID, HWND aWndCtl, BOOL &aHandled);
};

#endif // _EDIT_EVENT_DLG_H_
