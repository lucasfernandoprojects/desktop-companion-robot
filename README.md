# Cute Desktop Companion Robot

![Photo of the desktop companion robot over a green background.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-1.jpg)

*I've recently posted a tutorial about this project on YouTube. You can watch it [here](https://www.youtube.com/).*

Build your own cute desktop companion robot using an ESP32, a small display, a touch sensor, and a passive buzzer!

This little robot is designed to sit on your desk and interact with you throughout the day. It displays different facial expressions, plays cheerful melodies, and has a simple personality system that changes depending on how often you interact with it.

![Photo of the desktop companion robot over a desktop table.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-2.jpg)

![Photo of the desktop companion robot over a desktop table - close view.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-3.jpg)

The more you pet it, the happier it becomes. Leave it alone for too long, and it'll gradually get sleepy before eventually falling asleep to save battery power. Touch it again, and it'll wake up!

This is a fully open-source project, so feel free to download the files, build your own robot, and customize it however you like.

<div style="display: flex; flex-wrap: wrap;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-4.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-5.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-6.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-7.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-8.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-9.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-10.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-11.jpg" width="400" height="250" style="margin: 10px;">
</div>

## Features

- **Facial expressions:** Displays different emotions, including neutral, happy, enjoying, excited, sleepy, sleeping, and surprised.
- **Touch interaction:** Uses a TTP223 touch sensor to detect when you pet the robot.
- **Personality system:** The robot's mood changes depending on how frequently you interact with it.
- **Sound effects:** Plays different melodies through a passive buzzer depending on its current mood.
- **Automatic sleep:** Gradually becomes sleepy when left alone and eventually enters a sleeping state.
- **Battery saving:** Enters ESP32 light sleep mode after five minutes in the sleeping state without interaction.
- **Customizable design:** Includes the 3D models and Arduino code needed to build and modify the robot.

## Hardware Requirements

To build this project, you'll need the following components:

| Component | Description |
|---|---|
| ESP32 development board | Main microcontroller |
| I2C LCD display | Displays the robot's facial expressions |
| TTP223 touch sensor | Detects touch interactions |
| Passive buzzer | Plays melodies and sound effects |
| Li-ion battery | Powers the robot |
| On/off switch | Turns the robot on and off |
| TP4056 charging module (optional) | Required if your ESP32 board doesn't include a compatible battery charging circuit |
| Wires and heat-shrink tubing | For electrical connections and insulation |

<div style="display: flex; flex-wrap: wrap;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-12.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-13.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-14.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-15.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-16.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-17.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-18.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-19.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-27.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-28.jpg" width="400" height="250" style="margin: 10px;">
</div>

**Important:** Check the specifications of your ESP32 board before connecting a battery. Some development boards include a built-in battery charging circuit, while others don't. Make sure the battery voltage and connections are compatible with your board.

## Repository Structure

The repository is organized into four main folders:

```text
/
├── 3D-printing/
│   └── 3D models for printing the robot enclosure
│
├── code/
│   └── Arduino sketch
│
├── photos/
│   └── Project photos
│
└── schematics/
    └── Wiring diagrams
```

Here's what you'll find in each folder:

- **3D-printing:** Contains the files required to 3D print the robot's enclosure.
- **code:** Contains the Arduino sketch used to control the robot.
- **photos:** Contains pictures of the finished project and its assembly.
- **schematics:** Contains the wiring diagram showing how to connect all the electronic components.

## Assembly

Start by downloading the wiring diagram from the `/schematics` folder.

Connect the components according to the schematic, making sure to double-check the power connections before turning on the circuit.

The display and TTP223 touch sensor are powered by the ESP32's 3.3V output. Their GND connections are shared with the ESP32, and the passive buzzer also shares the same ground reference.

If you're using an ESP32 board with built-in battery charging, connect the battery to its designated battery connector. Otherwise, you'll need a compatible charging and power management solution.

Once all the components are connected, solder the wires and use heat-shrink tubing to insulate exposed connections.

Finally, download the 3D-printing files and assemble the electronics inside the enclosure.

<div style="display: flex; flex-wrap: wrap;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-20.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-21.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-22.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-23.jpg" width="400" height="250" style="margin: 10px;">
</div>

Additionally, you can follow the schematics below if you need more help wiring the components.

![Schematics of the circuit - ESP32 with battery charging.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/schematics/schematics-esp32-with-battery-charging.png
)

![Schematics of the circuit - ESP32 without battery charging.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/schematics/schematics-esp32-without-battery-charging.png
)

## Programming

To upload the firmware to your ESP32:

1. Download the Arduino sketch from the `/code` folder.
2. Open the `desktop_robot_companion` sketch in the Arduino IDE.
3. Connect your ESP32 development board to your computer.
4. Select the correct ESP32 board and COM port.
5. Click **Upload** and wait for the programming process to finish.

Once the upload is complete, disconnect the board from your computer and power up the robot.

## How It Works

The robot uses a simple personality system based on touch interactions and inactivity.

**Touch interactions:**

- **Short touch:** The robot smiles and plays a short, cheerful sound.
- **Long touch:** It closes its eyes and displays an expression as if it's enjoying being petted.
- **Repeated touches:** It becomes excited and plays a different melody.

**Inactivity:**

- After some time without interaction, the robot gradually becomes sleepy.
- Eventually, it enters its sleeping state.
- If there's no interaction for another five minutes, the ESP32 enters light sleep mode to reduce power consumption.

**Waking up:**

Touching the sensor wakes the robot up. It displays a surprised expression before returning to its normal state.

The melodies and facial expressions change according to the robot's current mood, making each interaction a little more interesting.

## 3D Printing Notes

The enclosure was designed to be 3D printed and assembled with the electronic components inside.

There are two versions of the enclosure design:

- **Updated version:** Designed for the standard ESP32 development board used in the final build.
- **Original version:** Includes an additional opening at the bottom, originally intended to provide access to the USB port and charging connection of an ESP32-S3 Super Mini.

During development, I had to switch from the ESP32-S3 Super Mini to a more standard ESP32 development board. That's why the original enclosure has an opening that isn't used in the final version.

I just included the updated version on this repository, but you are free to download and modify it the way you want.

## Customization

Feel free to modify this project and make the robot your own!

You can experiment with different facial expressions, change the melodies, adjust the personality system, or even redesign the enclosure.

The Arduino sketch is a good starting point if you want to add new behaviors or interactions.

## Sponsor

![Screenshot of Justway website - navigation menu.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-24.png)

This project was sponsored by **JustWay**, one of the best providers of manufacturing services for mechanical parts.

JustWay offers a wide range of manufacturing solutions, including 3D printing, injection molding, and CNC machining, making it a great partner for rapid prototyping and on-demand manufacturing.

Their website also features an instant quote tool, allowing you to estimate manufacturing costs without waiting for a manual quotation. Their ordering system is available 24 hours a day, 7 days a week.

![Screenshot of Justway website - services.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-25.png)

![Screenshot of Justway website - quote tool.](https://github.com/lucasfernandoprojects/desktop-companion-robot/blob/main/photos/photo-26.png)

A big thank you to JustWay for sponsoring this project and helping make STEM education more accessible!

Visit [JustWay's website](https://www.justway.com/) to learn more about their manufacturing services.

## License and Contributions

This project is completely open-source. You're welcome to download, modify, and build upon the provided files.

If you improve the design or come up with interesting new features, feel free to share your modifications with the maker community.

## Support My Work

If you enjoy building projects like this, consider subscribing to my YouTube channel, where I share tutorials to help makers build awesome stuff.

I also have a free guide covering **10 common ESP32 mistakes and how to fix them**. You can get it by joining my email list through this [link](https://lucas-fernando.kit.com/).

Thanks for checking out this project, and happy making!
