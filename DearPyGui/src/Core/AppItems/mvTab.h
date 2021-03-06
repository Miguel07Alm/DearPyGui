#pragma once

#include "Core/AppItems/mvTypeBases.h"
#include "mvApp.h"

//-----------------------------------------------------------------------------
// Widget Index
//
//     * mvTabBar
//     * mvTab
//
//-----------------------------------------------------------------------------

namespace Marvel {

	//-----------------------------------------------------------------------------
	// mvTabBar
	//-----------------------------------------------------------------------------
	class mvTabBar : public mvStringPtrBase
	{

	public:

		MV_APPITEM_TYPE(mvAppItemType::TabBar, "add_tab_bar")

		mvTabBar(const std::string& name)
			: mvStringPtrBase(name, "", name)
		{
			m_container = true;		
		}

		std::string& getValue() { return *m_value; }
		void setValue(const std::string& value) { *m_value = value; }

		void draw() override
		{
			pushColorStyles();
			ImGui::PushID(this);
			ImGui::BeginGroup();

			if (ImGui::BeginTabBar(m_label.c_str(), m_flags))
			{
				for (mvAppItem* item : m_children)
				{
					// skip item if it's not shown
					if (!item->isShown())
						continue;

					// set item width
					if (item->getWidth() != 0)
						ImGui::SetNextItemWidth((float)item->getWidth());

					item->pushColorStyles();
					item->draw();
					item->popColorStyles();

					item->getState().update();
				}

				ImGui::EndTabBar();
			}

			ImGui::EndGroup();
			ImGui::PopID();
			popColorStyles();
		}

		void setExtraConfigDict(PyObject* dict) override
		{	
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;

			// helper for bit flipping
			auto flagop = [dict](const char* keyword, int flag, int& flags)
			{
				if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
			};

			// window flags
			flagop("reorderable", ImGuiTabBarFlags_Reorderable, m_flags);

		}

		void getExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;

			// helper to check and set bit
			auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
			{
				PyDict_SetItemString(dict, keyword, ToPyBool(flags & flag));
			};

			// window flags
			checkbitset("reorderable", ImGuiTabBarFlags_Reorderable, m_flags);
		}

	private:

		ImGuiTabBarFlags m_flags = ImGuiTabBarFlags_None;

	};

	//-----------------------------------------------------------------------------
	// mvTab
	//-----------------------------------------------------------------------------
	class mvTab : public mvBoolPtrBase
	{

	public:

		MV_APPITEM_TYPE(mvAppItemType::TabItem, "add_tab")

		mvTab(const std::string& name)
			: mvBoolPtrBase(name, false, name)
		{
			m_container = true;
		}

		void draw() override
		{
			pushColorStyles();
			ImGui::PushID(this);

			// cast parent to mvTabBar
			auto parent = (mvTabBar*)m_parent;

			// check if this is first tab
			if (parent->getValue().empty())
			{
				// set mvTabBar value to the first tab name
				parent->setValue(m_name);
				*m_value = true;
			}

			// create tab item and see if it is selected
			if (ImGui::BeginTabItem(m_label.c_str(), m_closable ? &m_show : nullptr))
			{
				// Regular Tooltip (simple)
				if (!getTip().empty() && ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", getTip().c_str());

				parent->setValue(m_name);

				// set other tab's value false
				for (mvAppItem* child : parent->getChildren())
                    *((mvTab*)child)->m_value =false;

				// set current tab value true
				*m_value = true;

				//showAll();

				// run call back if it exists
				if (parent->getValue() != m_name)
				{
					mvApp::GetApp()->addCallback(parent->getCallback(), m_name, nullptr);

					// Context Menu
					if (!getPopup().empty())
						ImGui::OpenPopup(getPopup().c_str());
				}

				for (mvAppItem* item : m_children)
				{
					// skip item if it's not shown
					if (!item->isShown())
						continue;

					// set item width
					if (item->getWidth() != 0)
						ImGui::SetNextItemWidth((float)item->getWidth());

					item->pushColorStyles();
					item->draw();
					item->popColorStyles();

					item->getState().update();
				}

				ImGui::EndTabItem();
			}

			else
			{
				// Regular Tooltip (simple)
				if (!getTip().empty() && ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", getTip().c_str());
			}

			ImGui::PopID();
			popColorStyles();
		}

		void setExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;
			if (PyObject* item = PyDict_GetItemString(dict, "closable")) m_closable = ToBool(item);

		}

		void getExtraConfigDict(PyObject* dict) override
		{
			if (dict == nullptr)
				return;
			mvGlobalIntepreterLock gil;
			PyDict_SetItemString(dict, "closable", ToPyBool(m_closable));
		}

	private:

		bool m_closable = false;

	};

}