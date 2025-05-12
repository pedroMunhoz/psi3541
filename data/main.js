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

function updateDHT11() {
    fetch('/dht11', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.json())
    .then(data => {
        document.getElementById('temperature').innerText = `${data.temperature} °C`;
        document.getElementById('humidity').innerText = `${data.humidity} %`;
    })
    .catch(error => console.error('Error fetching DHT11 data:', error));
}

window.addEventListener('load', () => {
    updateLEDState();
    updateDHT11(); // Fetch temperature and humidity on page load
});