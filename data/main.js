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
    });
}

function sendCarConfig() {
    const Kp = parseFloat(document.getElementById('kp-value').value);
    const Kp_total = parseFloat(document.getElementById('kp-total-value').value);
    const Kd = parseFloat(document.getElementById('kd-value').value);

    fetch('/carConfig', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ Kp, Kp_total, Kd })
    })
    .then(res => res.json())
    .then(cfg => {
        document.getElementById('config-status').textContent = "Configuração enviada!";
        document.getElementById('kp-value').value = (Math.round(cfg.Kp * 100) / 100).toFixed(2);
        document.getElementById('kp-total-value').value = (Math.round(cfg.Kp_total * 100) / 100).toFixed(2);
        document.getElementById('kd-value').value = (Math.round(cfg.Kd * 100) / 100).toFixed(2);
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
        let txt = `
            <b>Estado:</b> ${carStateToText(status.state)}<br>
            <b>Referência:</b> ${(Math.round(status.ref * 100) / 100).toFixed(2)}<br>
            <b>Atual:</b> ${(Math.round(status.cur * 100) / 100).toFixed(2)}<br>
            <b>Finalizado:</b> ${status.done ? "Sim" : "Não"}
        `;
        document.getElementById('car-status-content').innerHTML = txt;
    })
    .catch(() => {
        document.getElementById('car-status-content').textContent = "Erro ao obter status.";
    });
}