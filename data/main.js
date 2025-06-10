function ledSet(state) {
    fetch('/ledSet', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ state: state }) // Send the LED state in the request body
    })
    .then(response => response.json())
    .then(data => {
        const state = data.state === 1 ? "ON" : "OFF";
        document.getElementById('gpio-state').innerText = state;
    })
    .catch(error => console.error('Error setting LED state:', error));
}

function updateLEDState() {
    fetch('/ledGet', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.json())
    .then(data => {
        const state = data.state === 1 ? "ON" : "OFF";
        document.getElementById('gpio-state').innerText = state;
    })
    .catch(error => console.error('Error fetching LED state:', error));
}

const temperatureData = [];
const humidityData = [];
const maxDataPoints = 100; // Maximum number of data points to display

let chart; // Chart.js instance

function initializeChart() {
    const ctx = document.getElementById('dht-chart').getContext('2d');
    chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [], // Time labels
            datasets: [
                {
                    label: 'Temperature (C)',
                    data: temperatureData,
                    borderColor: 'rgba(255, 99, 132, 1)',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    fill: true,
                },
                {
                    label: 'Humidity (%)',
                    data: humidityData,
                    borderColor: 'rgba(54, 162, 235, 1)',
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    fill: true,
                },
            ],
        },
        options: {
            responsive: true,
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Time',
                    },
                },
                y: {
                    title: {
                        display: true,
                        text: 'Value',
                    },
                    beginAtZero: true,
                },
            },
        },
    });
}

function updateDHT11() {
    fetch('/dht11', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.json())
    .then(data => {
        const temperature = data.temperature.toFixed(1);
        const humidity = data.humidity.toFixed(1);

        document.getElementById('temperature').textContent = `${temperature} C`;
        document.getElementById('humidity').textContent = `${humidity} %`;

        // Add data to the arrays
        const now = new Date().toLocaleTimeString();
        if (temperatureData.length >= maxDataPoints) {
            temperatureData.shift();
            humidityData.shift();
            chart.data.labels.shift();
        }
        temperatureData.push(temperature);
        humidityData.push(humidity);
        chart.data.labels.push(now);

        // Update the chart
        chart.update();
    })
    .catch(error => console.error('Error fetching DHT11 data:', error));
}

function setBlinkFrequency() {
    const frequency = document.getElementById('blink-frequency').value;

    fetch('/blink', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ frequency: parseInt(frequency, 10) }) // Send frequency in the request body
    })
    .then(response => response.json())
    .then(data => {
        console.log(`Blink frequency set to: ${frequency} Hz`);
    })
    .catch(error => console.error('Error setting blink frequency:', error));
}

function toggleBlink() {
    fetch('/blink', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: null // Empty body for toggle requests
    })
    .then(response => response.json())
    .then(data => {
        const state = data.state; // 1 for ON, 0 for OFF
        const button = document.getElementById('blink-toggle');
        const gpioState = document.getElementById('gpio-state');

        if (state === 1) {
            button.textContent = 'Turn Off Blink';
            button.classList.remove('button-green');
            button.classList.add('button-red');
            gpioState.textContent = `BLINKING`;
        } else {
            button.textContent = 'Start Blink';
            button.classList.remove('button-red');
            button.classList.add('button-green');
            gpioState.textContent = 'OFF';
        }
    })
    .catch(error => console.error('Error toggling blink state:', error));
}

function activateBlink() {
    fetch('/blink', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: null // Empty body to activate blink
    })
    .then(response => response.json())
    .then(data => {
        if (data.state === 1) { // Blink activated
            document.getElementById('led-menu').style.display = 'none';
            document.getElementById('blink-menu').style.display = 'block';
            document.getElementById('gpio-state').textContent = 'BLINKING';
        }
    })
    .catch(error => console.error('Error activating blink:', error));
}

function deactivateBlink() {
    fetch('/blink', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: null // Empty body to deactivate blink
    })
    .then(response => response.json())
    .then(data => {
        if (data.state === 0) { // Blink deactivated
            document.getElementById('blink-menu').style.display = 'none';
            document.getElementById('led-menu').style.display = 'block';
            document.getElementById('gpio-state').textContent = 'OFF';
        }
    })
    .catch(error => console.error('Error deactivating blink:', error));
}

// --- MQTT Task Control ---

function renderMqttTasksTable(data) {
    const table = document.getElementById('mqtt-tasks-table').getElementsByTagName('tbody')[0];
    table.innerHTML = '';
    const numTasks = data.num_tasks;

    // Store edited names in localStorage
    let taskNames = JSON.parse(localStorage.getItem('mqttTaskNames') || '{}');

    for (let i = 1; i <= numTasks; i++) {
        const active = data[i.toString()];
        const row = table.insertRow();

        // Editable name cell
        const nameCell = row.insertCell();
        const nameInput = document.createElement('input');
        nameInput.type = 'text';
        nameInput.value = taskNames[i] || `Task ${i}`;
        nameInput.className = 'mqtt-task-name-input'; // <-- add this line
        nameInput.onchange = () => {
            taskNames[i] = nameInput.value;
            localStorage.setItem('mqttTaskNames', JSON.stringify(taskNames));
        };
        nameCell.appendChild(nameInput);

        // Status+Switch cell
        const statusCell = row.insertCell();
        const label = document.createElement('label');
        label.className = 'switch';
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.checked = active;
        checkbox.onchange = () => mqttTaskControl(i, checkbox.checked ? 'start' : 'stop');
        const slider = document.createElement('span');
        slider.className = 'slider round';
        label.appendChild(checkbox);
        label.appendChild(slider);

        // Status text
        const statusText = document.createElement('span');
        statusText.textContent = active ? " Active" : " Inactive";
        statusText.style.marginLeft = "10px";

        statusCell.appendChild(label);
        statusCell.appendChild(statusText);
    }
}

function fetchMqttTasks() {
    fetch('/mqtt_tasks', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' }
    })
    .then(response => response.json())
    .then(data => renderMqttTasksTable(data))
    .catch(error => console.error('Error fetching MQTT tasks:', error));
}

function mqttTaskControl(idx, action) {
    fetch('/mqtt_task_control', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ idx: idx-1, action: action })
    })
    .then(() => fetchMqttTasks())
    .catch(error => console.error('Error controlling MQTT task:', error));
}

window.addEventListener('load', () => {
    updateLEDState();
    initializeChart();
    updateDHT11();
    fetchMqttTasks();
});


setInterval(updateDHT11, 5000);
// setInterval(updateLEDState, 500);
// setInterval(fetchMqttTasks, 2000);