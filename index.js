async function fetchData() {
    const response = await fetch('timeline.json');
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
            if (slot === '*')
                slotDiv.classList.add('filled');
            else if (slot === '.')
                slotDiv.classList.add('wait');
            slotsDiv.appendChild(slotDiv);
        });

        rowDiv.appendChild(processDiv);
        rowDiv.appendChild(slotsDiv);
        container.appendChild(rowDiv);
    });
}

fetchData().then(renderTimeline).catch(console.error);