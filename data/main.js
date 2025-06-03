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

function carMoveCommand(move) {
    fetch('/carMove', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ move: move })
    });
}

function carControl(move) {
    // Map button action to enum value
    const moveMap = {
        'front': 0,         // FRENTE
        'back': 1,          // TRAS
        'left': 2,          // ESQUERDA
        'right': 3,         // DIREITA
        'rot-left': 4,      // ROT_ESQUERDA
        'rot-right': 5,     // ROT_DIREITA
        'stop': 6           // PARAR
    };
    return moveMap[move];
}

// Attach event listeners after DOM is loaded
window.addEventListener('DOMContentLoaded', () => {
    document.querySelectorAll('.car-move-btn').forEach(btn => {
        const move = btn.getAttribute('data-move');
        btn.addEventListener('mousedown', () => carMoveCommand(carControl(move)));
        btn.addEventListener('touchstart', (e) => { e.preventDefault(); carMoveCommand(carControl(move)); });
        btn.addEventListener('mouseup', () => carMoveCommand(carControl('stop')));
        btn.addEventListener('mouseleave', () => carMoveCommand(carControl('stop')));
        btn.addEventListener('touchend', () => carMoveCommand(carControl('stop')));
    });
});

window.addEventListener('load', () => {
    fetchMqttTasks();
});