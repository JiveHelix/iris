#pragma once

#include <wxpex/async.h>
#include "user.h"
#include "display_error.h"


template<typename Actor>
class Observer
{
public:
    static constexpr auto observerName = "brain::Observer";

    Observer(Actor *actor, UserControl control)
        :
        actor_(actor),
        terminus_(this, control),
        doOpenFile_([this](){this->actor_->OpenFile();}),
        doLayoutWindows_([this](){this->actor_->LayoutWindows();})
    {
        this->terminus_.openFile.Connect(
            &Observer::OnOpenFile_);

        this->terminus_.saveSettings.Connect(
            &Observer::OnSaveSettings_);

        this->terminus_.loadSettings.Connect(
            &Observer::OnLoadSettings_);

        this->terminus_.about.Connect(
            &Observer::OnAbout_);

        this->terminus_.layoutWindows.Connect(
            &Observer::OnLayoutWindows_);

        this->terminus_.fileName.Connect(
            &Observer::OnFileName_);

        this->terminus_.errors.Connect(&Observer::OnErrors_);
    }

private:
    void OnFileName_(const std::string &)
    {
        this->doOpenFile_();
    }

    void OnOpenFile_()
    {
        // OpenFile will open windows that add new callbacks on this signal.
        // Do the work later, outside of this callback.
        this->doOpenFile_();
    }

    void OnLayoutWindows_()
    {
        // LayoutWindows will open windows that add new callbacks on this
        // signal.  Do the work later, outside of this callback.
        this->doLayoutWindows_();
    }

    void OnSaveSettings_()
    {
        this->actor_->SaveSettings();
    }

    void OnLoadSettings_()
    {
        this->actor_->LoadSettings();
    }

    void OnAbout_()
    {
        this->actor_->ShowAbout();
    }

    void OnErrors_(const std::string &errors)
    {
        DisplayError("Error", errors);
    }

private:
    Actor *actor_;
    UserTerminus<Observer<Actor>> terminus_;
    wxpex::CallAfter doOpenFile_;
    wxpex::CallAfter doLayoutWindows_;
};
