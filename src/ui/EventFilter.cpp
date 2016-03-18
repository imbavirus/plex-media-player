//
// Created by Tobias Hieta on 07/03/16.
//

#include "EventFilter.h"
#include "system/SystemComponent.h"
#include "settings/SettingsComponent.h"
#include "input/InputKeyboard.h"

#include <QKeyEvent>

///////////////////////////////////////////////////////////////////////////////////////////////////
bool EventFilter::eventFilter(QObject* watched, QEvent* event)
{
  SystemComponent& system = SystemComponent::Get();

  // ignore mouse events if mouse is disabled
  if  (SettingsComponent::Get().value(SETTINGS_SECTION_MAIN, "disablemouse").toBool() &&
       ((event->type() == QEvent::MouseMove) ||
        (event->type() == QEvent::MouseButtonPress) ||
        (event->type() == QEvent::MouseButtonRelease) ||
        (event->type() == QEvent::MouseButtonDblClick)))
  {
    return true;
  }

  if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
  {
    // In konvergo we intercept all keyboard events and translate them
    // into web client actions. We need to do this so that we can remap
    // keyboard buttons to different events.
    //

    bool pressed = event->type() == QEvent::KeyPress;

    QKeyEvent* kevent = dynamic_cast<QKeyEvent*>(event);
    if (kevent)
    {
      system.setCursorVisibility(false);
      if (kevent->spontaneous() && !kevent->isAutoRepeat())
      {
        // We ignore the KeypadModifier here since it's practically useless
        QKeySequence key(kevent->key() | (kevent->modifiers() &= ~Qt::KeypadModifier));
        InputKeyboard::Get().keyPress(key, pressed);
        return true;
      }
    }
  }
  else if (event->type() == QEvent::MouseMove)
  {
    system.setCursorVisibility(true);
  }
  else if (event->type() == QEvent::Wheel)
  {
    return true;
  }
  else if (event->type() == QEvent::MouseButtonPress)
  {
    // ignore right clicks that would show context menu
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if ((mouseEvent) && (mouseEvent->button() == Qt::RightButton))
      return true;
  }

  return QObject::eventFilter(watched, event);
}
