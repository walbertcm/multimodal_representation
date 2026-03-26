# Multimodal Representation: Visuo-Haptic Data Encoding

This repository contains a multimodal data visualization system developed for **visuo-haptic representation** research. The project features a D3.js-based web interface designed to map dataset variables (CSV) into visual stimuli and vibrotactile commands sent to an ESP32 microcontroller via WebSocket.

## 🚀 Features

* **Configurable Mapping**: Dynamically assign dataset columns to X/Y axes, color, and point size.
* **Haptic Encoding**: Map numerical variables to vibrotactile duration scales ranging from 50ms to 1000ms.
* **Binning Strategies**: Supports multiple data classification methods for haptic feedback:
    * Equal Interval Binning.
    * Quantile Binning.
    * **Geometric Scaling** (Default).
    * Z-score and Mean Split classifications.
* **Real-time Integration**: Low-latency WebSocket connection to trigger haptic actuators upon UI interaction.
* **Interaction Logging**: Automated logging and CSV export of all user interactions and sent commands for experimental analysis.

## 🛠️ Tech Stack

* **Frontend**: HTML5, CSS3, JavaScript (ES6+).
* **Visualization**: [D3.js v7](https://d3js.org/).
* **Communication**: WebSocket Protocol.
* **Hardware Compatibility**: ESP32 with LRA or ERM vibration actuators.

## 📋 Prerequisites

1.  **Hardware**: An ESP32 configured as a WebSocket server on the same local network.
2.  **Environment**: To load CSV files correctly, run the interface through a local server (e.g., VS Code *Live Server* extension) to avoid browser CORS restrictions.

## 📂 Project Structure

* `index.html`: The core visuo-haptic interface.
* `sprint4_data.csv`: Default dataset for testing and demonstration.
* `.gitignore`: Configured to exclude local environment settings and private directories.

## 📖 Research Context

This project is part of doctoral research focused on **Haptics and Data Visualization**, contributing to the "Brazil Haptics DataViz Dataset" for the IV 2026 conference.

The research is conducted by the **Applied Computing and Data Science Group** at IFPA Campus Bragança (ARCA).

## ⚖️ License

This project is licensed under the Apache 2.0 License.
