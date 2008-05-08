/*
AMIS: Adaptive Multimedia Information System
Software for playing DAISY books
Homepage: http://amis.sf.net

Copyright (C) 2004-2007  DAISY for All Project

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"
#include "gui/dialogs/SearchForBooksDialog.h"
#include "../resource.h"
#include "util/SearchForFilesMFC.h"
#include "AmisCore.h"
#include "Preferences.h"
#include "gui/dialogs/PathDialog.h"

#include "gui/self-voicing/datamodel/DataTree.h"

using namespace amis::gui::dialogs;

BEGIN_MESSAGE_MAP(SearchForBooksDialog, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_STARTSEARCH, OnStartsearch)
	ON_BN_CLICKED(IDC_STOPSEARCH, OnStopsearch)
	ON_LBN_DBLCLK(IDC_FILESFOUND, OnDblclkFilelist)
	ON_BN_CLICKED(IDC_OPENBOOK, OnOpenbook)
	ON_LBN_SELCHANGE(IDC_FILESFOUND, OnSelchangeFilelist)
END_MESSAGE_MAP()


void SearchForBooksDialog::OnSelchangeFilelist() 
{
	if (amis::Preferences::Instance()->getIsSelfVoicing() == true)
	{
		//TODO: this list treatment removes the default XML prompts, needs improvement
		CListBox* p_filelist = NULL;
		p_filelist = (CListBox*)this->GetDlgItem(IDC_FILESFOUND);

		int sel = p_filelist->GetCurSel();
		if (sel >= 0) { 
			amis::UrlList* p_search_results = mSearcher.getSearchResults();
			if (sel > -1 && sel < p_search_results->size()) {
				//mLoadBookOnDialogClose = (*p_search_results)[sel];
				AudioSequence * seq = new AudioSequence();
				CString str;
				p_filelist->GetText(sel, str);
				seq->append(str);
				AudioSequencePlayer::Instance()->Play(seq);
			}
		}
	}
}
void SearchForBooksDialog::resolvePromptVariables(Prompt* pPrompt) {
	
	PromptVar* p_var = NULL;
	PromptItem* promptNotAvailable = DataTree::Instance()->findPromptItem("not_available");

	for (int i=0; i<pPrompt->getNumberOfItems(); i++)
	{
		if (pPrompt->getItem(i)->getPromptItemType() == PROMPT_VARIABLE)
		{
			p_var = (PromptVar*)pPrompt->getItem(i);

			if (p_var->getName().compare("NUM_FILES_FOUND") == 0)
			{
				CString value;
				value.Format(_T("%d"), mFilesFound);
				wstring str;
				str = value;
				p_var->setContents(str, "");
			}
		}
	}
	AmisDialogBase::resolvePromptVariables(pPrompt);
}

SearchForBooksDialog::SearchForBooksDialog(CWnd* pParent /*=NULL*/)
	: AmisDialogBase(SearchForBooksDialog::IDD)
{
	mCaptionWhileSearching.LoadStringW(IDS_SEARCHING);
	mCaptionDefault.LoadStringW(AFX_IDS_IDLEMESSAGE);
	mCaptionSearchAborted.LoadStringW(IDS_SEARCH_STOPPED);
	mCaptionSearchCompleteNoFilesFound.LoadStringW(IDS_NO_FILES_FOUND);
	mCaptionSearchCompleteOneFileFound.LoadStringW(IDS_ONE_FILE_FOUND);
	mCaptionSearchCompleteFilesFound.LoadStringW(IDS_FILES_FOUND);
	mFilesFound = 0;

	mbShouldStopSearching = false;
}

SearchForBooksDialog::~SearchForBooksDialog()
{
}

BOOL SearchForBooksDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CButton* p_button = (CButton*)this->GetDlgItem(IDC_STOPSEARCH);
	p_button->EnableWindow(FALSE);

	p_button = (CButton*)this->GetDlgItem(IDC_OPENBOOK);
	p_button->EnableWindow(FALSE);
	
	return TRUE;
}


void SearchForBooksDialog::announceStatus(CString msg, SearchStatus status)
{
	CWnd* p_label_widget = NULL;
	p_label_widget = this->GetDlgItem(IDC_SEARCHING);
	if (p_label_widget)
	{
		p_label_widget->SetWindowText(msg);
		p_label_widget->ShowWindow(SW_SHOW);
	}
	if (amis::Preferences::Instance()->getIsSelfVoicing() == true) 
	{
		if (status == SEARCHING) 
		{
			AudioSequencePlayer::Instance()->playDialogControlFromUiIds(IDD_SEARCHDAISY, IDC_SEARCHING, this, true, "WhileSearching");
		} 
		else if (status == NONE_FOUND) 
		{
			AudioSequencePlayer::Instance()->playDialogControlFromUiIds(IDD_SEARCHDAISY, IDC_SEARCHING, this, true, "SearchCompleteNoFilesFound");
		} 
		else if (status == ONE_FOUND || status == MANY_FOUND) 
		{
			CListBox* p_filelist = NULL;
			p_filelist = (CListBox*)this->GetDlgItem(IDC_FILESFOUND);
			p_filelist->SetCurSel(0);

			AudioSequence * seq = NULL;
			if (status == ONE_FOUND) 
			{
				seq = AudioSequencePlayer::Instance()->playDialogControlFromUiIds(IDD_SEARCHDAISY, IDC_SEARCHING, this, false, "SearchCompleteOneFileFound");
			} 
			else 
			{
				seq = AudioSequencePlayer::Instance()->playDialogControlFromUiIds(IDD_SEARCHDAISY, IDC_SEARCHING, this, false, "SearchCompleteFilesFound");
			}
		
			AudioSequence * seq2 = AudioSequencePlayer::Instance()->playDialogControlFromUiIds(IDD_SEARCHDAISY, IDC_FILESFOUND, this, false, "default");
			seq->appendAll(seq2);
			delete seq2;
		
			int sel = p_filelist->GetCurSel();
			if (sel >= 0) 
			{ 
				amis::UrlList* p_search_results = mSearcher.getSearchResults();
				if (sel > -1 && sel < p_search_results->size()) 
				{
					//mLoadBookOnDialogClose = (*p_search_results)[sel];	
					CString str;
					p_filelist->GetText(sel, str);
					seq->append(str);
				}
			}
			AudioSequencePlayer::Instance()->Play(seq);

		} 
		else if (status == STOPPED) 
		{
			AudioSequencePlayer::Instance()->playDialogControlFromUiIds(IDD_SEARCHDAISY, IDC_SEARCHING, this, true, "SearchAborted");
		} 
	}
}

//TODO: fill in titles, not URLs
void SearchForBooksDialog::populateListControl()
{
	USES_CONVERSION;
	CListBox* p_filelist = NULL;
	p_filelist = (CListBox*)this->GetDlgItem(IDC_FILESFOUND);

	amis::UrlList* p_search_results = mSearcher.getSearchResults();
	amis::UrlList::iterator it;
	int i = 0;
	for (it = p_search_results->begin(); it != p_search_results->end(); ++it)
	{
		CString result;
		result = A2T(it->get_file().c_str());
		p_filelist->AddString(result);
		i++;
	}
	
	p_filelist->SetFocus();

	if (p_search_results->size() == 0)
	{
		CEdit* p_edit = (CEdit*)this->GetDlgItem(IDC_SEARCHPATH);
		p_edit->SetWindowText(_T(""));
		p_edit->SetFocus();
	}
	else
	{
		CButton* p_button = (CButton*)this->GetDlgItem(IDC_OPENBOOK);
		p_button->EnableWindow(TRUE);
	}
}

void SearchForBooksDialog::loadBook()
{
	CListBox* p_list = NULL;
	p_list = (CListBox*)this->GetDlgItem(IDC_FILESFOUND);
	int sel = p_list->GetCurSel();
	amis::UrlList* p_search_results = mSearcher.getSearchResults();
	if (sel > -1 && sel < p_search_results->size()) mLoadBookOnDialogClose = (*p_search_results)[sel];
	this->EndDialog(1);
}

ambulant::net::url SearchForBooksDialog::getBookToLoad()
{
	return mLoadBookOnDialogClose;
}

void SearchForBooksDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	// Let the window do its default painting...
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)dc.m_hDC, 0 );
	//call the base class on_paint function
	on_paint();
}

void SearchForBooksDialog::OnBrowse() 
{
	CString sel_folder_title;
	CString sel_folder_instr;
	CString curr_path;

	sel_folder_title.LoadString(IDS_SELFOLDER_TITLE);
	sel_folder_instr.LoadString(IDS_SELFOLDER_INSTR);

	CPathDialog path_dlg(sel_folder_title, sel_folder_instr, curr_path, NULL);
	if (path_dlg.DoModal() == IDOK)
	{
		curr_path = path_dlg.GetPathName();
		CEdit* p_edit = NULL;
		p_edit = (CEdit*)this->GetDlgItem(IDC_SEARCHPATH);
		p_edit->SetWindowText(curr_path);
	}
}

void SearchForBooksDialog::OnStartsearch() 
{
	USES_CONVERSION;
	CButton* p_button = (CButton*)this->GetDlgItem(IDC_OPENBOOK);
	p_button->EnableWindow(FALSE);

	mbShouldStopSearching = false;
	
	//clear the file list
	CListBox* p_filelist = NULL;
	p_filelist = (CListBox*)this->GetDlgItem(IDC_FILESFOUND);
	int sz = p_filelist->GetCount();
	for (int i=sz - 1; i>=0; i--) p_filelist->DeleteString(0);
	
	CEdit* p_edit = NULL;
	p_edit = (CEdit*)this->GetDlgItem(IDC_SEARCHPATH);
	CString search_string;
	p_edit->GetWindowText(search_string);
	search_string.TrimLeft();
	search_string.TrimRight();

	if (search_string.IsEmpty() == TRUE) return;
	
	//change the text on the side to say "searching"
	announceStatus(mCaptionWhileSearching, SEARCHING);

	//gray-out the search button and enable the stop button
	p_button = (CButton*)this->GetDlgItem(IDC_STARTSEARCH);
	p_button->EnableWindow(FALSE);

	p_button = (CButton*)this->GetDlgItem(IDC_STOPSEARCH);
	p_button->EnableWindow(TRUE);

	//prepare the search tool
	mSearcher.clearAll();
	mSearcher.addSearchCriteria("ncc.htm");
	mSearcher.addSearchCriteria(".opf");
	//sometimes I see these temp files on my drive .. excluding them just to be safe
	mSearcher.addSearchExclusionCriteria("_ncc.html");

	// SVN folders are interfering
	mSearcher.addSearchExclusionCriteria(".svn-base");

	mSearcher.setRecursiveSearch(true);
	//start the search
	mFilesFound = mSearcher.startSearch(T2A(search_string));
	populateListControl();

	if (mFilesFound == 0) announceStatus(this->mCaptionSearchCompleteNoFilesFound, NONE_FOUND);
	else if (mFilesFound == 1) announceStatus(this->mCaptionSearchCompleteOneFileFound, ONE_FOUND);
	else
	{
		CString temp;
		temp.Format(mCaptionSearchCompleteFilesFound, mFilesFound);
		announceStatus(temp, MANY_FOUND);
	}

	//reverse the gray-out button states
	p_button = (CButton*)this->GetDlgItem(IDC_STARTSEARCH);
	p_button->EnableWindow(TRUE);

	p_button = (CButton*)this->GetDlgItem(IDC_STOPSEARCH);
	p_button->EnableWindow(FALSE);
}


void SearchForBooksDialog::OnStopsearch() 
{
	mbShouldStopSearching = true;
	mSearcher.stopSearch();
	announceStatus(mCaptionSearchAborted, STOPPED);
}

void SearchForBooksDialog::OnOpenbook() 
{
	loadBook();
}

void SearchForBooksDialog::OnDblclkFilelist() 
{
	loadBook();
}

BOOL SearchForBooksDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		CWnd* p_wnd = this->GetFocus();
		
		if (p_wnd)
		{
			int id = p_wnd->GetDlgCtrlID();

			if (id == IDC_FILESFOUND && (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN))
			{
				return CDialog::PreTranslateMessage(pMsg);
			}

			//inexplicably, the default behavior for pressing enter in a dialog is to close it.
			//we're overriding this here.
			if (pMsg->wParam == VK_RETURN)
			{
				CEdit* p_edit =	(CEdit*)GetDlgItem(IDC_SEARCHPATH);

				//if the textbox has focus and the user pressed enter, start searching
				if (id == IDC_SEARCHPATH && pMsg->hwnd == p_wnd->m_hWnd && p_edit->m_hWnd == pMsg->hwnd) // making sure that we're not capturing events from the Japanese IME
				 {
					pMsg->wParam = NULL;
					OnStartsearch();
					return CDialog::PreTranslateMessage(pMsg);
				 }
				 //if the files list has focus, open the selected book
				 else if (id == IDC_FILESFOUND)
				 {
					 pMsg->wParam = NULL;
					 loadBook();
					 return CDialog::PreTranslateMessage(pMsg);
				 }
			}	
		}
	}
	return AmisDialogBase::PreTranslateMessageTextField(pMsg, IDC_SEARCHPATH);
}
