async function fetchData() {
    const response = await fetch('stats.json');
    return response.json();
}

function renderTimeline(data) {
    const container = document.getElementById('timeline');
    data.forEach(row => {
        const rowDiv = document.createElement('div');
        rowDiv.classList.add('row');

        const processDiv = document.createElement('div');
        processDiv.classList.add('process');
        processDiv.textContent = row.process;

        const slotsDiv = document.createElement('div');
        slotsDiv.classList.add('slots');

        row.slots.forEach(slot => {
            const slotDiv = document.createElement('div');
            slotDiv.classList.add('slot');
            slotDiv.textContent = slot; // Set the text content of the slot
            slotsDiv.appendChild(slotDiv);
        });

        rowDiv.appendChild(processDiv);
        rowDiv.appendChild(slotsDiv);
        container.appendChild(rowDiv);
    });
}

fetchData().then(renderTimeline).catch(console.error);
