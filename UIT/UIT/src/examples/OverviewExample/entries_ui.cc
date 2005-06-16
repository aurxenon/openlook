/*
 * entries_ui.cc - User interface object initialization functions.
 * This file was generated by `guic' from `entries.G'.
 */

#include "BaseWindow.h"
#include "ComponentDisplay.h"
#include "Button.h"
#include "ListChoice.h"
#include "AlphanumericInput.h"
#include "Notifier.h"

extern void add_button_handler (UIObject *);
extern void entry_list_handler (UIObject *);
extern void delete_button_handler (UIObject *);

int main (int argc, char **argv)
{
  BaseWindow entry_window;
  entry_window.initUI (argc, argv);
  entry_window.setWidth (307);
  entry_window.setHeight (132);
  entry_window.setLabel ("Entries");
  entry_window.show (TRUE);
  entry_window.setDisplayFooter (TRUE);
  entry_window.setResizable (TRUE);

  ComponentDisplay entry_display (TRUE);
  entry_display.setHelp ("entries:entry_display");
  entry_display.setX (0);
  entry_display.setY (0);
  entry_display.setExtendToEdge (WIDTH);
  entry_display.setExtendToEdge (HEIGHT);
  entry_display.setDisplayBorders (FALSE);
  entry_window.addDisplay (entry_display);

  Button add_button;
  add_button.setHelp ("entries:add_button");
  add_button.setX (8);
  add_button.setY (16);
  add_button.setLabel ("Add");
  add_button.setNotifyHandler (add_button_handler);
  entry_display.addComponent (add_button);

  ListChoice entry_list;
  entry_list.setHelp ("entries:entry_list");
  entry_list.setX (76);
  entry_list.setY (16);
  entry_list.setWidth (200);
  entry_list.setDisplayedRows (3);
  entry_list.setLabelPosition (LEFT);
  entry_list.setReadOnly (FALSE);
  entry_list.setMultipleChoice (FALSE);
  entry_list.setSelectionRequired (FALSE);
  entry_list.setNotifyHandler (entry_list_handler);
  entry_display.addComponent (entry_list);

  Button delete_button;
  delete_button.setHelp ("entries:delete_button");
  delete_button.setX (8);
  delete_button.setY (48);
  delete_button.setLabel ("Delete");
  delete_button.setNotifyHandler (delete_button_handler);
  entry_display.addComponent (delete_button);

  AlphanumericInput entry_field;
  entry_field.setHelp ("entries:entry_field");
  entry_field.setX (8);
  entry_field.setY (104);
  entry_field.setInputDisplayLength (30);
  entry_field.setMaxInputLength (30);
  entry_field.setLabel ("Entry:");
  entry_field.setLabelPosition (LEFT);
  entry_field.setReadOnly (FALSE);
  entry_display.addComponent (entry_field);

  Notifier notifier;
  notifier.start ();

  exit (0);
}
