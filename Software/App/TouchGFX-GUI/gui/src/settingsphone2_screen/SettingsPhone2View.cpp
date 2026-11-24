#include <gui/settingsphone2_screen/SettingsPhone2View.hpp>

SettingsPhone2View::SettingsPhone2View()
{

}

void SettingsPhone2View::setupScreen()
{
    SettingsPhone2ViewBase::setupScreen();

    menu.setTitle(T_TEXT_PHONE_UC);
    menu.setInfoMsgColor(Gui::Config::Color::MENU_NOT_SELECTED_TIP_ON);

    menu.setNumberOfItems(Gui::Config::Menu::Top::Settings::Phone::ID_COUNT);

    MenuItemSelected *pS = nullptr;
    MenuItemNotSelected *pN = nullptr;

    // PAIR
    pS = menu.getSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_PAIR);
    pS->config(T_TEXT_PAIR_ANOTHER);

    pN = menu.getNotSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_PAIR);
    pN->config(T_TEXT_PAIR_ANOTHER);

    // HR BROADCAST
    touchgfx::Unicode::strncpy(mHrBroadcastLabel, "HR Broadcast", skLabelSize - 1);
    pS = menu.getSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_HR_BROADCAST);
    pS->configToggle(mHrBroadcastLabel);
    pS->setMsgTypedTextId(T_TMP_SEMIBOLD_25);
    hrBroadcast(mHrBroadcast);

    menu.invalidate();
}

void SettingsPhone2View::bonded(uint8_t count)
{
    menu.setInfoMsg(count, count == 1 ? T_TEXT_DEVICE_PAIRED : T_TEXT_DEVICES_PAIRED);

    TypedTextId unpair = count > 1 ? T_TEXT_UNPAIR_ALL : T_TEXT_UNPAIR;

    // UNPAIR
    MenuItemSelected *pS = menu.getSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_UNPAIR);
    pS->config(unpair);

    MenuItemNotSelected *pN = menu.getNotSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_UNPAIR);
    pN->config(unpair);

    menu.invalidate();
}

void SettingsPhone2View::tearDownScreen()
{
    SettingsPhone2ViewBase::tearDownScreen();
}

void SettingsPhone2View::connected(bool connected)
{
    mConnected = connected;
    updateItems();
}

void SettingsPhone2View::hrBroadcast(bool on)
{
    mHrBroadcast = on;

    MenuItemSelected *pS = menu.getSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_HR_BROADCAST);
    MenuItemNotSelected *pN = menu.getNotSelectedItem(Gui::Config::Menu::Top::Settings::Phone::ID_HR_BROADCAST);

    pS->setToggle(on);

    pN->configTip(mHrBroadcastLabel, on ? T_TEXT_ON_UC : T_TEXT_OFF_UC);
    pN->setTipColor(on ? Gui::Config::Color::MENU_NOT_SELECTED_TIP_ON : Gui::Config::Color::MENU_NOT_SELECTED_TIP_OFF);

    updateItems();
}

void SettingsPhone2View::updateItems()
{
    // "Pair Another" is the last item: hide it while a phone is connected, and
    // while broadcasting, which keeps phones away.
    if (mConnected || mHrBroadcast) {
        if (menu.getSelectedItem() >= Gui::Config::Menu::Top::Settings::Phone::ID_PAIR) {
            menu.selectItem(Gui::Config::Menu::Top::Settings::Phone::ID_UNPAIR);
        }
        menu.setNumberOfItems(Gui::Config::Menu::Top::Settings::Phone::ID_PAIR);
    } else {
        menu.setNumberOfItems(Gui::Config::Menu::Top::Settings::Phone::ID_COUNT);
    }
    menu.invalidate();
}

void SettingsPhone2View::handleKeyEvent(uint8_t key)
{
    if (key == Gui::Config::Button::L1) {
        menu.selectPrev();
    }

    if (key == Gui::Config::Button::L2) {
        menu.selectNext();
    }

    if (key == Gui::Config::Button::R1) {
        uint32_t id = menu.getSelectedItem();

        switch (id) {
            case Gui::Config::Menu::Top::Settings::Phone::ID_UNPAIR:
                presenter->unpair();
                application().gotoSettingsPhone1ScreenNoTransition();
                break;
            case Gui::Config::Menu::Top::Settings::Phone::ID_PAIR:
                presenter->startPairing();
                application().gotoPairingScreenNoTransition();
                break;
            case Gui::Config::Menu::Top::Settings::Phone::ID_HR_BROADCAST:
                // Shown at once; the backend's answer corrects it if the
                // broadcast could not start
                hrBroadcast(!mHrBroadcast);
                presenter->setHrBroadcast(mHrBroadcast);
                break;
            default:
                break;
        };
    }

    if (key == Gui::Config::Button::R2) {
        application().gotoTopMenuScreenNoTransition();
    }
}