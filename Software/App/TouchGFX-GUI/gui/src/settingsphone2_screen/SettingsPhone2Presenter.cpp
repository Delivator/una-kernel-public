#include <gui/settingsphone2_screen/SettingsPhone2View.hpp>
#include <gui/settingsphone2_screen/SettingsPhone2Presenter.hpp>

SettingsPhone2Presenter::SettingsPhone2Presenter(SettingsPhone2View& v)
    : view(v)
{

}

void SettingsPhone2Presenter::activate()
{
    // Reset idle timer
    model->resetIdleTimer();

    view.bonded(model->bleGetBondedCount());
    view.connected(model->bleIsConnected());
    view.hrBroadcast(model->hrBroadcastIsOn());
}

void SettingsPhone2Presenter::deactivate()
{

}

void SettingsPhone2Presenter::unpair()
{
    model->bleUnpair();
}

void SettingsPhone2Presenter::onBleBond(bool paired)
{
    view.bonded(model->bleGetBondedCount());
}

void SettingsPhone2Presenter::onBleConnect(bool connected)
{
    view.connected(connected);
}

void SettingsPhone2Presenter::onHrBroadcast(bool on)
{
    view.hrBroadcast(on);
}

void SettingsPhone2Presenter::onHrBroadcastStrapInUse()
{
    view.strapInUse();
}

void SettingsPhone2Presenter::setHrBroadcast(bool on)
{
    model->hrBroadcastSet(on);
}

void SettingsPhone2Presenter::startPairing()
{
    if (!model->bleIsPairingActive()) {
        model->bleStartPairing();
    }
}