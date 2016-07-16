# Bongo Controller for Dark Souls

## Synopsis

This project provides a method to play Dark Souls on PC using a Donkey Konga bongo drum controller
as an input method, as well as a GUI for displaying on stream. Example: https://www.youtube.com/watch?v=4HdWWhZ0hDM

## About the bongo controller

This project is designed for the bongo drum controller used by various Donkey Kong Gamecube games. A Gamecube to USB
adapter is required to use it on PC. The bongo controller has 5 buttons (2 on each drum and one Start button on the front),
as well as one microphone clap sensor (analog input).

## Controlling Dark Souls

The 6 inputs provided by the bongos are not enough to beat Dark Souls with a basic setup. To solve this problem, the
controller interface cycles through three different states, with each state having a different set of inputs.

| State | Start         | Left bongo top | Left bongo bottom  | Right bongo top | Right bongo bottom   | Clap    |
|-------|---------------|----------------|--------------------|-----------------|----------------------|---------|
| 1     | Move forward  | Light attack   | Rotate camera left | Roll            | Rotate camera right  | Lock on |
| 2     | Move backward | Parry          | Block              | Two hand        | Use item             | Gesture |
| 3     | Use           | Dpad down      | Dpad left          | Dpad up         | Dpad right           | Menu    |

To move forward one state, both bottom buttons must be pressed simultaneously. To move back one state, both top
buttons must be pressed simultaneously.

## Requirements

* Built with wxWidgets 3.1.0, but will likely work with older versions
* Built with Visual Studio 2015, but will likely work with older versions
* You will need a Gamecube -> USB adapter to connect the bongo controller to a Windows PC. These are easy to find on Amazon etc.
* The GUID of the bongo controller is currently hardcoded into BongoController.cpp, and must be changed to the GUID of your
particular bongo controller in order to be used