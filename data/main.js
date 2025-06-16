// --- MQTT Task Control ---

let selectedState = 0; // Default: Frente

document.addEventListener('DOMContentLoaded', () => {
    // Highlight selected button
    document.querySelectorAll('.car-move-menu-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.car-move-menu-btn').forEach(b => b.classList.remove('pressed'));
            this.classList.add('pressed');
            selectedState = parseInt(this.getAttribute('data-state'));
            // Atualiza unidade de referência conforme o comando
            let unit = (selectedState === 0 || selectedState === 1 || selectedState === 2 || selectedState === 3) ? 'cm' : 'graus';
            document.getElementById('ref-unit').textContent = unit;
        });
    });

    // Send command
    document.getElementById('send-move-btn').addEventListener('click', () => {
        let ref = parseFloat(document.getElementById('ref-value').value) || 0;
        ref = Math.round(ref * 100) / 100; // até 2 casas decimais
        fetch('/carMove', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ state: selectedState, ref: ref })
        })
        .then(res => {
            if (res.ok) {
                document.getElementById('move-status').textContent = "Comando enviado!";
            } else {
                document.getElementById('move-status').textContent = "Erro ao enviar comando.";
            }
        })
        .catch(() => {
            document.getElementById('move-status').textContent = "Erro ao enviar comando.";
        });
    });

    // Configuração PD
    fetchCarConfig();
    document.getElementById('send-config-btn').addEventListener('click', sendCarConfig);

    // Status do carrinho
    fetchCarStatus();
    setInterval(fetchCarStatus, 250);
});

function fetchCarConfig() {
    fetch('/carConfig', {
        method: 'POST'
    })
    .then(res => res.json())
    .then(cfg => {
        document.getElementById('kp-value').value = (Math.round(cfg.Kp * 100) / 100).toFixed(2);
        document.getElementById('kp-total-value').value = (Math.round(cfg.Kp_total * 100) / 100).toFixed(2);
        document.getElementById('kd-value').value = (Math.round(cfg.Kd * 100) / 100).toFixed(2);
        document.getElementById('ki-value').value = (Math.round(cfg.Ki * 100) / 100).toFixed(2);
    });
}

function sendCarConfig() {
    const Kp = parseFloat(document.getElementById('kp-value').value);
    const Kp_total = parseFloat(document.getElementById('kp-total-value').value);
    const Kd = parseFloat(document.getElementById('kd-value').value);
    const Ki = parseFloat(document.getElementById('ki-value').value);

    fetch('/carConfig', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ Kp, Kp_total, Kd, Ki })
    })
    .then(res => res.json())
    .then(cfg => {
        document.getElementById('config-status').textContent = "Configuração enviada!";
        document.getElementById('kp-value').value = (Math.round(cfg.Kp * 100) / 100).toFixed(2);
        document.getElementById('kp-total-value').value = (Math.round(cfg.Kp_total * 100) / 100).toFixed(2);
        document.getElementById('kd-value').value = (Math.round(cfg.Kd * 100) / 100).toFixed(2);
        document.getElementById('ki-value').value = (Math.round(cfg.Ki * 100) / 100).toFixed(2);
    })
    .catch(() => {
        document.getElementById('config-status').textContent = "Erro ao enviar configuração.";
    });
}

function carStateToText(state) {
    switch (state) {
        case 0: return "Frente";
        case 1: return "Trás";
        case 2: return "Esquerda";
        case 3: return "Direita";
        case 4: return "Rotacionar Esquerda";
        case 5: return "Rotacionar Direita";
        case 6: return "Parado";
        default: return "Desconhecido";
    }
}

function fetchCarStatus() {
    fetch('/carStatus', {
        method: 'POST'
    })
    .then(res => res.json())
    .then(status => {
        // Texto do status
        let txt = `
            <b>Estado:</b> ${carStateToText(status.state)}<br>
            <b>Referência:</b> ${(Math.round(status.ref * 100) / 100).toFixed(2)}<br>
            <b>Atual:</b> ${(Math.round(status.cur * 100) / 100).toFixed(2)}<br>
            <b>Finalizado:</b> ${status.done ? "Sim" : "Não"}
        `;
        document.getElementById('car-status-content').innerHTML = txt +
        `<div style="margin-top:12px;">
            <div id="progress-bar-container" style="background:#e0e0e0; border-radius:8px; height:18px; width:100%; max-width:220px; margin:0 auto; overflow:hidden;">
                <div id="progress-bar" style="background:#008c3a; height:100%; width:0%; transition:width 0.3s;"></div>
            </div>
            <div id="progress-text" style="font-size:0.95em; margin-top:4px; color:#333;"></div>
        </div>
        <canvas id="car-canvas" width="180" height="80" style="margin-top:12px; display:block; margin-left:auto; margin-right:auto;"></canvas>`;

        // Barra de progresso
        let perc = 0;
        if (status.ref !== 0) {
            perc = Math.max(0, Math.min(1, status.cur / status.ref));
        }
        document.getElementById('progress-bar').style.width = (perc * 100).toFixed(1) + "%";
        document.getElementById('progress-text').textContent = `Progresso: ${(perc * 100).toFixed(1)}%`;

        // Desenho do carrinho
        drawCarMovement(status.state, status.ref, status.cur);
    })
    .catch(() => {
        document.getElementById('car-status-content').textContent = "Erro ao obter status.";
    });
}

// Desenha o carrinho no canvas de acordo com o movimento
function drawCarMovement(state, ref, cur) {
    const canvas = document.getElementById('car-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Parâmetros visuais
    const carW = 28, carH = 16;
    ctx.save();

    if (state === 0 || state === 1 || state === 2 || state === 3) {
        // Movimento retilíneo (frente, trás, esquerda, direita)
        // Linha de referência
        ctx.strokeStyle = "#bbb";
        ctx.lineWidth = 4;
        ctx.beginPath();
        ctx.moveTo(20, canvas.height/2);
        ctx.lineTo(canvas.width-20, canvas.height/2);
        ctx.stroke();

        // Carrinho
        let x = 20 + ((canvas.width-40) * Math.max(0, Math.min(1, cur/ref || 0)));
        ctx.translate(x, canvas.height/2);
        ctx.fillStyle = "#034078";
        ctx.fillRect(-carW/2, -carH/2, carW, carH);
        ctx.strokeStyle = "#222";
        ctx.strokeRect(-carW/2, -carH/2, carW, carH);
        // Roda
        ctx.fillStyle = "#222";
        ctx.fillRect(-carW/2, -carH/2-4, 8, 4);
        ctx.fillRect(carW/2-8, -carH/2-4, 8, 4);
        ctx.fillRect(-carW/2, carH/2, 8, 4);
        ctx.fillRect(carW/2-8, carH/2, 8, 4);
    } else if (state === 4 || state === 5) {
        // Rotação: desenha círculo e carrinho girando
        const cx = canvas.width/2, cy = canvas.height/2+8, r = 28;
        ctx.strokeStyle = "#bbb";
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, 2*Math.PI);
        ctx.stroke();

        // Ângulo atual
        let ang = 0;
        if (ref !== 0) ang = 2 * Math.PI * (cur/ref);
        if (state === 5) ang = -ang; // Rotacionar Direita

        ctx.translate(cx, cy);
        ctx.rotate(ang);
        ctx.fillStyle = "#034078";
        ctx.fillRect(r-carW/2, -carH/2, carW, carH);
        ctx.strokeStyle = "#222";
        ctx.strokeRect(r-carW/2, -carH/2, carW, carH);
        // Roda
        ctx.fillStyle = "#222";
        ctx.fillRect(r-carW/2, -carH/2-4, 8, 4);
        ctx.fillRect(r+carW/2-8, -carH/2-4, 8, 4);
        ctx.fillRect(r-carW/2, carH/2, 8, 4);
        ctx.fillRect(r+carW/2-8, carH/2, 8, 4);
    }
    ctx.restore();
}


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
    fetchMqttTasks();
});