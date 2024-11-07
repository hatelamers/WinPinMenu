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
		return m_pFirstElement && m_pSearchConditionButton;
	}

	template<typename TFunc>
	HRESULT ForEveryButton(TFunc itemReceiver)
	{
		if (!IsReady())
		{
			ATLTRACE2(_T("CTaskbarAutomation hasn't been initialed\n"));
			return E_FAIL;
		}
		CComPtr<IUIAutomationElementArray> pTaskbarButtonElements;
		auto hr = m_pFirstElement->FindAll(TreeScope_Descendants, m_pSearchConditionButton, &pTaskbarButtonElements);
		if (SUCCEEDED(hr))
		{
			auto count = 0;
			hr = pTaskbarButtonElements->get_Length(&count);
			for (auto i = 0; SUCCEEDED(hr) && i < count; i++)
			{
				CComPtr<IUIAutomationElement> pButton;
				hr = pTaskbarButtonElements->GetElement(i, &pButton);
				if (SUCCEEDED(hr) && !itemReceiver(pButton, i))
				{
					hr = S_FALSE;
					break;
				}
			}
		}
		return hr;
	}

protected:
    bool Initialize()
    {
		if (m_pFirstElement && m_pSearchConditionButton)
			return true;

		auto hr = S_OK;
		if (!m_pAutomation)
		{
			hr = m_pAutomation.CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER);
			//HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&m_pAutomation);
			if (FAILED(hr)) return false;
		}

		CComPtr<IUIAutomationElement> pRootElement;
		hr = m_pAutomation->GetRootElement(&pRootElement);
		if (FAILED(hr)) return false;

		CComPtr<IUIAutomationCondition> pTaskbarCondition;
		hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"MSTaskListWClass"), &pTaskbarCondition);
		if (SUCCEEDED(hr))
		{
			hr = pRootElement->FindFirst(TreeScope_Descendants, pTaskbarCondition, &m_pFirstElement);
			if (SUCCEEDED(hr) && m_pFirstElement)
			{
				hr = m_pAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId, CComVariant(UIA_ButtonControlTypeId), &m_pSearchConditionButton);
				if (FAILED(hr)) return false;
			}
		}

		if (!m_pFirstElement)
		{
			if (pTaskbarCondition)
			{
				pTaskbarCondition.Release();
			}
			hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"Shell_TrayWnd"), &pTaskbarCondition);
			if (FAILED(hr)) return false;

			hr = pRootElement->FindFirst(TreeScope_Children, pTaskbarCondition, &m_pFirstElement);
			if (FAILED(hr) || !m_pFirstElement) return false;


			hr = m_pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"Taskbar.TaskListButtonAutomationPeer"), &m_pSearchConditionButton);
			if (FAILED(hr)) return false;
		}

		return true;

    }

protected:

    CComPtr<IUIAutomation> m_pAutomation;
    CComPtr<IUIAutomationElement> m_pFirstElement;
    CComPtr<IUIAutomationCondition> m_pSearchConditionButton;

};
