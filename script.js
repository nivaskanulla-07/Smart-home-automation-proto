// ============================================================
// ESP32 CONFIGURATION
// ============================================================

const ESP32_IP = "192.168.0.8";


// ============================================================
// HTML ELEMENTS
// ============================================================

const switches = [
    ...document.querySelectorAll(".switch-card")
];

const allOnButton =
    document.getElementById("allOn");

const allOffButton =
    document.getElementById("allOff");

const roomStatus =
    document.getElementById("roomStatus");


// ============================================================
// DEVICE CONFIGURATION
// ============================================================
//
// SWITCH 1 -> RELAY 1 -> FAN
// SWITCH 2 -> RELAY 2 -> SPEAKER
// SWITCH 3 -> TEMPERATURE DISPLAY
// SWITCH 4 -> CLOCK
//
// ============================================================

const devices = {

    0: {
        name: "Fan",
        type: "relay",
        relay: 1
    },

    1: {
        name: "Speaker",
        type: "relay",
        relay: 2
    },

    2: {
        name: "Temperature",
        type: "none",
        relay: null
    },

    3: {
        name: "Clock",
        type: "clock",
        relay: null
    }

};


// ============================================================
// SEND RELAY COMMAND
// ============================================================

async function sendRelayCommand(relay, isOn) {

    const command =
        isOn ? "on" : "off";

    const url =
        `http://${ESP32_IP}/relay${relay}/${command}`;

    console.log("--------------------------------");
    console.log("RELAY COMMAND");
    console.log("Relay:", relay);
    console.log("Command:", command);
    console.log("URL:", url);
    console.log("--------------------------------");


    try {

        const response =
            await fetch(url);


        if (!response.ok) {

            throw new Error(
                `HTTP ${response.status}`
            );
        }


        const result =
            await response.text();


        console.log(
            "ESP32 response:",
            result
        );


        return true;

    }

    catch (error) {

        console.error(
            "Relay error:",
            error
        );


        alert(
            "Unable to connect to ESP32.\n\n" +
            "Check:\n" +
            "• ESP32 is powered ON\n" +
            "• ESP32 IP is 192.168.0.7\n" +
            "• Both devices are on the same Wi-Fi"
        );


        return false;
    }

}


// ============================================================
// SEND CLOCK COMMAND
// ============================================================

async function sendClockCommand(isOn) {

    const command =
        isOn ? "on" : "off";

    const url =
        `http://${ESP32_IP}/clock/${command}`;


    console.log("--------------------------------");
    console.log("CLOCK COMMAND");
    console.log("Command:", command);
    console.log("URL:", url);
    console.log("--------------------------------");


    try {

        const response =
            await fetch(url);


        if (!response.ok) {

            throw new Error(
                `HTTP ${response.status}`
            );
        }


        const result =
            await response.text();


        console.log(
            "Clock response:",
            result
        );


        return true;

    }

    catch (error) {

        console.error(
            "Clock connection error:",
            error
        );


        alert(
            "Unable to connect to the Clock.\n\n" +
            "Check the Clock ESP32 command."
        );


        return false;
    }

}


// ============================================================
// CHANGE SWITCH VISUAL STATE
// ============================================================

function setSwitchState(card, isOn) {

    card.classList.toggle(
        "on",
        isOn
    );


    card.setAttribute(
        "aria-pressed",
        String(isOn)
    );


    const status =
        card.querySelector("small");


    if (status) {

        status.textContent =
            isOn ? "On" : "Off";
    }


    if (
        typeof updateAllButtonColours ===
        "function"
    ) {

        updateAllButtonColours();
    }

}


// ============================================================
// UPDATE ROOM STATUS
// ============================================================

function updateInterface() {

    const activeDevices =
        switches.filter(
            card => card.classList.contains("on")
        );


    const onCount =
        activeDevices.length;


    if (onCount === 0) {

        roomStatus.textContent =
            "All Devices Off";

    }

    else if (
        onCount === switches.length
    ) {

        roomStatus.textContent =
            "All Devices On";

    }

    else {

        roomStatus.textContent =
            `${onCount} Device${onCount > 1 ? "s" : ""} On`;
    }

}


// ============================================================
// TEMPERATURE DISPLAY
// ============================================================
//
// DHT11 runs continuously on the ESP32.
//
// Switch 3 ONLY controls whether the website
// requests/displays the temperature.
//
// ESP32:
// http://192.168.0.7/temperature
//
// ============================================================

let temperatureTimer = null;


// ============================================================
// ADD TEMPERATURE COLOUR STYLES
// ============================================================

(function addTemperatureStyles() {

    if (
        document.getElementById(
            "temperature-colour-styles"
        )
    ) {

        return;
    }


    const style =
        document.createElement("style");


    style.id =
        "temperature-colour-styles";


    style.textContent = `

        .switch-card.temperature-blue {
            background: #2196f3 !important;
            border-color: #1976d2 !important;
            color: white !important;
        }

        .switch-card.temperature-yellow {
            background: #ffd600 !important;
            border-color: #f9a825 !important;
            color: #111 !important;
        }

        .switch-card.temperature-orange {
            background: #ff9800 !important;
            border-color: #ef6c00 !important;
            color: white !important;
        }

        .switch-card.temperature-red {
            background: #f44336 !important;
            border-color: #c62828 !important;
            color: white !important;
        }

        .switch-card.temperature-blue small,
        .switch-card.temperature-orange small,
        .switch-card.temperature-red small {
            color: white !important;
        }

        .switch-card.temperature-yellow small {
            color: #111 !important;
        }

        #allOn.active {
            background: #22c55e !important;
            color: white !important;
        }

        #allOff.active {
            background: #ef4444 !important;
            color: white !important;
        }

    `;


    document.head.appendChild(style);

})();


// ============================================================
// SET TEMPERATURE COLOUR
// ============================================================

function setTemperatureColour(value) {

    const card =
        switches[2];


    if (!card) return;


    card.classList.remove(
        "temperature-blue",
        "temperature-yellow",
        "temperature-orange",
        "temperature-red"
    );


    // BELOW 25°C -> BLUE
    if (value < 25) {

        card.classList.add(
            "temperature-blue"
        );
    }


    // 25°C TO 30°C -> YELLOW
    else if (value <= 30) {

        card.classList.add(
            "temperature-yellow"
        );
    }


    // ABOVE 30°C TO 40°C -> ORANGE
    else if (value <= 40) {

        card.classList.add(
            "temperature-orange"
        );
    }


    // ABOVE 40°C -> RED
    else {

        card.classList.add(
            "temperature-red"
        );
    }

}


// ============================================================
// CLEAR TEMPERATURE COLOUR
// ============================================================

function clearTemperatureColour() {

    const card =
        switches[2];


    if (!card) return;


    card.classList.remove(
        "temperature-blue",
        "temperature-yellow",
        "temperature-orange",
        "temperature-red"
    );

}


// ============================================================
// REQUEST TEMPERATURE
// ============================================================

async function getTemperature() {

    const temperatureCard =
        switches[2];


    if (!temperatureCard) {

        console.error(
            "Temperature card not found."
        );

        return;
    }


    try {

        const url =
            `http://${ESP32_IP}/temperature`;


        console.log("--------------------------------");
        console.log("TEMPERATURE REQUEST");
        console.log("URL:", url);
        console.log("--------------------------------");


        const response =
            await fetch(
                url,
                {
                    method: "GET",
                    cache: "no-store"
                }
            );


        if (!response.ok) {

            throw new Error(
                `HTTP ${response.status}`
            );
        }


        const data =
            await response.json();


        console.log(
            "Temperature response:",
            data
        );


        const value =
            Number(data.temperature);


        if (!Number.isFinite(value)) {

            throw new Error(
                "Invalid temperature value received"
            );
        }


        const status =
            temperatureCard.querySelector(
                "small"
            );


        if (status) {

            status.textContent =
                `${value.toFixed(1)}°C`;
        }


        // Change switch colour.
        setTemperatureColour(value);

    }


    catch (error) {

        console.error(
            "Temperature request failed:",
            error
        );


        const status =
            temperatureCard.querySelector(
                "small"
            );


        if (status) {

            status.textContent =
                "Unavailable";
        }

    }

}


// ============================================================
// START TEMPERATURE DISPLAY
// ============================================================

function startTemperatureDisplay() {

    // Prevent duplicate timers.
    stopTemperatureDisplay();


    const temperatureCard =
        switches[2];


    if (!temperatureCard) return;


    const status =
        temperatureCard.querySelector(
            "small"
        );


    if (status) {

        status.textContent =
            "Reading...";
    }


    // Request immediately.
    getTemperature();


    // Update every 2.5 seconds.
    temperatureTimer =
        setInterval(
            getTemperature,
            2500
        );

}


// ============================================================
// STOP TEMPERATURE DISPLAY
// ============================================================

function stopTemperatureDisplay() {

    if (temperatureTimer !== null) {

        clearInterval(
            temperatureTimer
        );

        temperatureTimer = null;
    }


    clearTemperatureColour();

}


// ============================================================
// UPDATE ALL ON / ALL OFF BUTTON COLOURS
// ============================================================

function updateAllButtonColours() {

    if (
        !allOnButton ||
        !allOffButton
    ) {

        return;
    }


    const allOn =
        switches.length > 0 &&
        switches.every(
            card =>
                card.classList.contains("on")
        );


    const allOff =
        switches.length > 0 &&
        switches.every(
            card =>
                !card.classList.contains("on")
        );


    allOnButton.classList.toggle(
        "active",
        allOn
    );


    allOffButton.classList.toggle(
        "active",
        allOff
    );

}


// ============================================================
// INDIVIDUAL SWITCH CONTROL
// ============================================================

switches.forEach(
    (card, index) => {

        card.addEventListener(
            "click",
            async () => {

                const device =
                    devices[index];


                // --------------------------------------------
                // SAFETY CHECK
                // --------------------------------------------

                if (!device) {

                    console.error(
                        "Unknown switch:",
                        index + 1
                    );

                    return;
                }


                // --------------------------------------------
                // CURRENT STATE
                // --------------------------------------------

                const currentState =
                    card.classList.contains("on");


                const newState =
                    !currentState;


                let success = false;


                // ============================================
                // SWITCH 1 / 2
                // RELAYS
                // ============================================

                if (
                    device.type === "relay"
                ) {

                    success =
                        await sendRelayCommand(
                            device.relay,
                            newState
                        );
                }


                // ============================================
                // SWITCH 4
                // CLOCK
                // ============================================

                else if (
                    device.type === "clock"
                ) {

                    success =
                        await sendClockCommand(
                            newState
                        );
                }


                // ============================================
                // SWITCH 3
                // TEMPERATURE DISPLAY ONLY
                // ============================================

                else if (
                    device.name ===
                    "Temperature"
                ) {

                    if (newState) {

                        startTemperatureDisplay();

                    }

                    else {

                        stopTemperatureDisplay();


                        const status =
                            card.querySelector(
                                "small"
                            );


                        if (status) {

                            status.textContent =
                                "Off";
                        }
                    }


                    // No relay command.
                    success = true;
                }


                // --------------------------------------------
                // UPDATE UI AFTER SUCCESS
                // --------------------------------------------

                if (success) {

                    setSwitchState(
                        card,
                        newState
                    );


                    updateInterface();


                    updateAllButtonColours();


                    console.log(
                        `${device.name}:`,
                        newState
                            ? "ON"
                            : "OFF"
                    );
                }

            }
        );

    }
);


// ============================================================
// ALL ON
// ============================================================

if (allOnButton) {

    allOnButton.addEventListener(
        "click",
        async () => {

            console.log(
                "========== ALL ON =========="
            );


            // --------------------------------------------
            // RELAY 1 -> ON
            // --------------------------------------------

            const relay1 =
                await sendRelayCommand(
                    1,
                    true
                );


            // --------------------------------------------
            // RELAY 2 -> ON
            // --------------------------------------------

            const relay2 =
                await sendRelayCommand(
                    2,
                    true
                );


            // --------------------------------------------
            // TEMPERATURE -> DISPLAY ON
            // --------------------------------------------

            startTemperatureDisplay();


            setSwitchState(
                switches[2],
                true
            );


            // --------------------------------------------
            // CLOCK -> ON
            // --------------------------------------------

            const clock =
                await sendClockCommand(
                    true
                );


            // --------------------------------------------
            // UPDATE SWITCH 1
            // --------------------------------------------

            if (relay1) {

                setSwitchState(
                    switches[0],
                    true
                );
            }


            // --------------------------------------------
            // UPDATE SWITCH 2
            // --------------------------------------------

            if (relay2) {

                setSwitchState(
                    switches[1],
                    true
                );
            }


            // --------------------------------------------
            // UPDATE SWITCH 4
            // --------------------------------------------

            if (clock) {

                setSwitchState(
                    switches[3],
                    true
                );
            }


            updateInterface();

            updateAllButtonColours();

        }
    );

}


// ============================================================
// ALL OFF
// ============================================================

if (allOffButton) {

    allOffButton.addEventListener(
        "click",
        async () => {

            console.log(
                "========== ALL OFF =========="
            );


            // --------------------------------------------
            // RELAY 1 -> OFF
            // --------------------------------------------

            const relay1 =
                await sendRelayCommand(
                    1,
                    false
                );


            // --------------------------------------------
            // RELAY 2 -> OFF
            // --------------------------------------------

            const relay2 =
                await sendRelayCommand(
                    2,
                    false
                );


            // --------------------------------------------
            // TEMPERATURE -> DISPLAY OFF
            // --------------------------------------------

            stopTemperatureDisplay();


            setSwitchState(
                switches[2],
                false
            );


            // --------------------------------------------
            // CLOCK -> OFF
            // --------------------------------------------

            const clock =
                await sendClockCommand(
                    false
                );


            // --------------------------------------------
            // UPDATE SWITCH 1
            // --------------------------------------------

            if (relay1) {

                setSwitchState(
                    switches[0],
                    false
                );
            }


            // --------------------------------------------
            // UPDATE SWITCH 2
            // --------------------------------------------

            if (relay2) {

                setSwitchState(
                    switches[1],
                    false
                );
            }


            // --------------------------------------------
            // UPDATE SWITCH 4
            // --------------------------------------------

            if (clock) {

                setSwitchState(
                    switches[3],
                    false
                );
            }


            updateInterface();

            updateAllButtonColours();

        }
    );

}


// ============================================================
// STARTUP
// ============================================================

// DHT11 continues running on ESP32.
// Website temperature display starts OFF.

stopTemperatureDisplay();


switches.forEach(
    card => {

        setSwitchState(
            card,
            false
        );

    }
);


updateInterface();

updateAllButtonColours();


// ============================================================
// ESP32 CONNECTION TEST
// ============================================================

async function testESP32() {

    console.log(
        "Testing ESP32..."
    );


    try {

        const response =
            await fetch(
                `http://${ESP32_IP}/`
            );


        const result =
            await response.text();


        console.log(
            "ESP32 CONNECTED!"
        );


        console.log(
            "ESP32 response:",
            result
        );


        return true;

    }

    catch (error) {

        console.error(
            "ESP32 CONNECTION FAILED",
            error
        );


        return false;
    }

}