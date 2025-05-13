function ledSet(state) {
    fetch('/led', {
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
    fetch('/led', {
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

window.addEventListener('load', () => {
    updateLEDState();
    initializeChart();
    updateDHT11();
});