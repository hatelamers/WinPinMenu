#pragma once

#include <uiautomation.h>

//#pragma comment(lib, "uiautomationcore.lib")

class CTaskbarAutomation
{
public:
    CTaskbarAutomation()
    {
		Initialize();
    }

    virtual ~CTaskbarAutomation() = default;

	bool IsReady() const noexcept
	{
		return m_pTaskbars && m_pSearchConditionButton;
	}

	template<typename TFunc>
	HRESULT ForEveryButton(TFunc itemReceiver)
	{
		if (!IsReady())
		{
			ATLTRACE2(_T("CTaskbarAutomation hasn't been initialed\n"));
			return E_FAIL;
		}

		auto tbCount = 0;
		auto hr = m_pTaskbars->get_Length(&tbCount);
		ATLTRACE(__FUNCTION__ _T(" %d taskbars\n"), tbCount);
		for (auto i = 0; SUCCEEDED(hr) && i < tbCount; i++)
		{
			CComPtr<IUIAutomationElement> pTaskbar;
			hr = m_pTaskbars->GetElement(i, &pTaskbar);
			if (SUCCEEDED(hr))
			{
				CComPtr<IUIAutomationElementArray> pTaskbarButtonElements;
				hr = pTaskbar->FindAllBuildCache(TreeScope_Descendants, m_pSearchConditionButton, m_pCacheRequestButton, &pTaskbarButtonElements);
				if (SUCCEEDED(hr))
				{
					auto btnCount = 0;
					hr = pTaskbarButtonElements->get_Length(&btnCount);
					for (auto j = 0; SUCCEEDED(hr) && j < btnCount; j++)
					{
						CComPtr<IUIAutomationElement> pButton;
						hr = pTaskbarButtonElements->GetElement(j, &pButton);
						if (SUCCEEDED(hr) && !itemReceiver(pButton, i, j))
						{
							hr = S_FALSE;
							break;
						}
					}
				}

			}
		}

		return hr;
	}

protected:
    bool Initialize()
    {
		ATLTRACE(__FUNCTION__ _T(" START\n"));
		if (m_pTaskbars && m_pSearchConditionButton)
			return true;

		auto hr = S_OK;
		if (!m_pAutomation)
		{
			hr = m_pAutomation.CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER);
			if (FAILED(hr)) return false;
		}

		CComPtr<IUIAutomationElement> pRootElement;
		hr = m_pAutomation->GetRootElement(&pRootElement);
		if (FAILED(hr)) return false;

		CComPtr<IUIAutomationCondition> pPrimTaskbarCondition;
		hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"Shell_TrayWnd"), &pPrimTaskbarCondition);
		if (FAILED(hr)) return false;

		CComPtr<IUIAutomationCondition> pSecondTaskbarCondition;
		hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"Shell_SecondaryTrayWnd"), &pSecondTaskbarCondition);
		if (FAILED(hr)) return false;

		CComPtr<IUIAutomationCondition> pTaskbarCondition;
		hr = m_pAutomation->CreateOrCondition(pPrimTaskbarCondition, pSecondTaskbarCondition, &pTaskbarCondition);
		if (FAILED(hr)) return false;

		hr = pRootElement->FindAll(TreeScope_Children, pTaskbarCondition, &m_pTaskbars);
		if (FAILED(hr) || !m_pTaskbars) return false;

		//CComPtr<IUIAutomationCondition> pTaskListCondition;
		//hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"MSTaskListWClass"), &pTaskListCondition);
		//if (FAILED(hr)) return false;


		CComPtr<IUIAutomationCondition> pSearchConditionUXButton;
		hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"Taskbar.TaskListButtonAutomationPeer"), &pSearchConditionUXButton);
		if (FAILED(hr)) return false;

		CComPtr<IUIAutomationCondition> pSearchConditionButton;
		hr = m_pAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId, CComVariant(UIA_ButtonControlTypeId), &pSearchConditionButton);
		if (FAILED(hr)) return false;

		hr = m_pAutomation->CreateOrCondition(pSearchConditionButton, pSearchConditionUXButton, &m_pSearchConditionButton);
		if (FAILED(hr)) return false;

		hr = m_pAutomation->CreateCacheRequest(&m_pCacheRequestButton);
		if (FAILED(hr)) return false;

		hr = m_pCacheRequestButton->AddProperty(UIA_BoundingRectanglePropertyId);
		if (FAILED(hr)) return false;

		ATLTRACE(__FUNCTION__ _T(" END\n"));
		return true;

    }

protected:

    CComPtr<IUIAutomation> m_pAutomation;
    CComPtr<IUIAutomationElementArray> m_pTaskbars;
    CComPtr<IUIAutomationCondition> m_pSearchConditionButton;
	CComPtr<IUIAutomationCacheRequest> m_pCacheRequestButton;

};
