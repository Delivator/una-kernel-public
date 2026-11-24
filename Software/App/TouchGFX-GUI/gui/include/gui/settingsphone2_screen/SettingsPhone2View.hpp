#ifndef SETTINGSPHONE2VIEW_HPP
#define SETTINGSPHONE2VIEW_HPP

#include <gui_generated/settingsphone2_screen/SettingsPhone2ViewBase.hpp>
#include <gui/settingsphone2_screen/SettingsPhone2Presenter.hpp>

class SettingsPhone2View : public SettingsPhone2ViewBase
{
public:
    SettingsPhone2View();
    virtual ~SettingsPhone2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void connected(bool connected);
    void bonded(uint8_t count);
    void hrBroadcast(bool on);

protected:
    virtual void handleKeyEvent(uint8_t key) override;

private:
    // "HR Broadcast" is not in the text database: the menu items' wildcard
    // texts show it from this buffer.
    static constexpr uint16_t skLabelSize = 16;
    touchgfx::Unicode::UnicodeChar mHrBroadcastLabel[skLabelSize] {};

    bool mConnected = false;
    bool mHrBroadcast = false;

    void updateItems();
};

#endif // SETTINGSPHONE2VIEW_HPP
