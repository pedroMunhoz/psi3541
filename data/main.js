// --- MQTT Task Control ---

let selectedState = 0; // Default: Frente

document.addEventListener('DOMContentLoaded', () => {
    // Highlight selected button
    document.querySelectorAll('.car-move-menu-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.car-move-menu-btn').forEach(b => b.classList.remove('pressed'));
            this.classList.add('pressed');
            selectedState = parseInt(this.getAttribute('data-state'));
            // Update unit
            let unit = (selectedState <= 1) ? 'cm' : 'graus';
            document.getElementById('ref-unit').textContent = unit;
        });
    });

    // Send command
    document.getElementById('send-move-btn').addEventListener('click', () => {
        const ref = parseInt(document.getElementById('ref-value').value) || 0;
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
});