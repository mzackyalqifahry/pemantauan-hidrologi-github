<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Pemantauan Hidrologi - Update 10 Detik</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body {
      font-family: Arial, sans-serif;
      padding: 20px;
      background: #f0f4f8;
    }
    h1 {
      text-align: center;
      color: #1e3a8a;
      margin-bottom: 5px;
    }
    .subtitle {
      text-align: center;
      color: #666;
      margin-bottom: 20px;
    }
    .dashboard {
      display: flex;
      flex-wrap: wrap;
      gap: 20px;
      margin-bottom: 20px;
    }
    .card {
      background: white;
      border-radius: 10px;
      padding: 15px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      flex: 1;
      min-width: 250px;
    }
    .card h2 {
      margin-top: 0;
      color: #1e3a8a;
      font-size: 1.2em;
    }
    .status {
      font-size: 1.5em;
      font-weight: bold;
      text-align: center;
      padding: 10px;
      border-radius: 5px;
      margin: 10px 0;
    }
    .status-aman { background-color: #e6f7e6; color: green; }
    .status-siaga { background-color: #fff3e6; color: orange; }
    .status-bahaya { background-color: #ffebee; color: red; }
    .sensor-value {
      font-size: 2em;
      font-weight: bold;
      text-align: center;
      margin: 10px 0;
      color: #1e3a8a;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 20px;
      background: white;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
    }
    th, td {
      padding: 12px;
      border-bottom: 1px solid #ddd;
      text-align: center;
    }
    canvas {
      margin-top: 30px;
      background: white;
      padding: 10px;
      border-radius: 10px;
      width: 100%;
    }
    .last-update {
      text-align: right;
      font-size: 0.9em;
      color: #666;
      margin-bottom: 10px;
    }
    .sensor-info {
      display: flex;
      justify-content: space-between;
      margin-top: 15px;
    }
    .sensor-info div {
      text-align: center;
      flex: 1;
    }
    .refresh-rate {
      background-color: #e3f2fd;
      padding: 3px 8px;
      border-radius: 10px;
      font-size: 0.8em;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Sistem Pemantauan Hidrologi</h1>
    <div class="subtitle">Update data setiap <span class="refresh-rate">10 detik</span></div>
    <div class="last-update" id="last-update">Terakhir diperbarui: -</div>
    
    <div class="dashboard">
      <div class="card">
        <h2>Status Ketinggian Air</h2>
        <div id="current-status" class="status status-aman">-</div>
        <div class="sensor-value">
          <span id="current-water-level">-</span> cm
        </div>
        <div class="sensor-info">
          <div>Jarak Sensor: <span id="current-distance">-</span> cm</div>
          <div>Tinggi Tangki: 250 cm</div>
        </div>
      </div>
      
      <div class="card">
        <h2>Informasi Threshold</h2>
        <div><span class="status-aman">Aman</span>: &lt;150 cm</div>
        <div><span class="status-siaga">Siaga</span>: 150-180 cm</div>
        <div><span class="status-bahaya">Bahaya</span>: &gt;180 cm</div>
        <div style="margin-top: 15px;">
          <strong>Sensor HC-SR04</strong><br>
          Akurasi: ±3mm<br>
          Range: 2-400cm
        </div>
      </div>
    </div>

    <table>
      <thead>
        <tr>
          <th>Waktu</th>
          <th>Tinggi Air (cm)</th>
          <th>Jarak (cm)</th>
          <th>Status</th>
        </tr>
      </thead>
      <tbody id="data-table">
        <!-- Data akan diisi JavaScript -->
      </tbody>
    </table>

    <canvas id="waterChart" height="200"></canvas>
  </div>

  <script>
    // Data dari Arduino
    let waterData = [];
    let chart;
    let lastUpdateTime = 0;

    // Inisialisasi chart
    function initChart() {
      const ctx = document.getElementById('waterChart').getContext('2d');
      chart = new Chart(ctx, {
        type: 'line',
        data: {
          labels: [],
          datasets: [
            {
              label: 'Tinggi Air (cm)',
              data: [],
              backgroundColor: 'rgba(59, 130, 246, 0.2)',
              borderColor: 'rgba(37, 99, 235, 1)',
              borderWidth: 2,
              tension: 0.3,
              fill: true,
              yAxisID: 'y'
            },
            {
              label: 'Jarak Sensor (cm)',
              data: [],
              backgroundColor: 'rgba(255, 159, 64, 0.2)',
              borderColor: 'rgba(255, 159, 64, 1)',
              borderWidth: 2,
              tension: 0.3,
              fill: false,
              yAxisID: 'y1'
            }
          ]
        },
        options: {
          responsive: true,
          animation: {
            duration: 500 // Animasi lebih cepat untuk update sering
          },
          scales: {
            y: {
              type: 'linear',
              display: true,
              position: 'left',
              title: { display: true, text: 'Tinggi Air (cm)' },
              max: 250,
              min: 0
            },
            y1: {
              type: 'linear',
              display: true,
              position: 'right',
              title: { display: true, text: 'Jarak Sensor (cm)' },
              max: 250,
              min: 0,
              grid: { drawOnChartArea: false }
            },
            x: {
              title: { display: true, text: 'Waktu' },
              ticks: {
                maxRotation: 45,
                minRotation: 45
              }
            }
          },
          plugins: {
            annotation: {
              annotations: {
                warningLine: {
                  type: 'line',
                  yMin: 150,
                  yMax: 150,
                  borderColor: 'orange',
                  borderWidth: 1.5,
                  borderDash: [4, 4],
                  label: {
                    content: 'Siaga',
                    enabled: true,
                    position: 'right',
                    backgroundColor: 'rgba(255, 159, 64, 0.7)'
                  }
                },
                dangerLine: {
                  type: 'line',
                  yMin: 180,
                  yMax: 180,
                  borderColor: 'red',
                  borderWidth: 1.5,
                  borderDash: [4, 4],
                  label: {
                    content: 'Bahaya',
                    enabled: true,
                    position: 'right',
                    backgroundColor: 'rgba(255, 99, 132, 0.7)'
                  }
                }
              }
            },
            tooltip: {
              mode: 'index',
              intersect: false
            }
          }
        }
      });
    }

    // Update tampilan dengan data baru
    function updateDisplay(data) {
      const now = new Date();
      lastUpdateTime = now.getTime();
      
      // Update card status
      document.getElementById('current-status').textContent = data.status;
      document.getElementById('current-status').className = `status status-${data.status.toLowerCase()}`;
      document.getElementById('current-water-level').textContent = data.tinggi.toFixed(1);
      document.getElementById('current-distance').textContent = data.jarak.toFixed(1);
      
      // Update waktu terakhir
      document.getElementById('last-update').textContent = 
        `Terakhir diperbarui: ${now.toLocaleTimeString()}`;
      
      // Tambahkan data ke tabel
      addToTable(data);
      
      // Update chart
      updateChart(data);
    }

    function addToTable(data) {
      const tbody = document.getElementById('data-table');
      const row = document.createElement('tr');
      
      row.innerHTML = `
        <td>${data.waktu}</td>
        <td>${data.tinggi.toFixed(1)}</td>
        <td>${data.jarak.toFixed(1)}</td>
        <td class="status-${data.status.toLowerCase()}">${data.status}</td>
      `;
      
      tbody.insertBefore(row, tbody.firstChild);
      
      // Batasi hanya 15 data terbaru
      if (tbody.children.length > 15) {
        tbody.removeChild(tbody.lastChild);
      }
    }

    function updateChart(data) {
      waterData.push(data);
      
      // Batasi data chart untuk 30 titik terakhir
      if (waterData.length > 30) {
        waterData.shift();
      }
      
      chart.data.labels = waterData.map(d => d.waktu);
      chart.data.datasets[0].data = waterData.map(d => d.tinggi);
      chart.data.datasets[1].data = waterData.map(d => d.jarak);
      chart.update();
    }

    // Simulasi data untuk demo
    function simulateData() {
      const now = new Date();
      const timeStr = now.getHours().toString().padStart(2, '0') + ':' + 
                     now.getMinutes().toString().padStart(2, '0') + ':' + 
                     now.getSeconds().toString().padStart(2, '0');
      
      // Simulasi pembacaan sensor dengan sedikit variasi
      const baseDistance = 30 + Math.sin(Date.now()/10000) * 15;
      const randomVariation = (Math.random() - 0.5) * 5;
      const distance = Math.max(2, Math.min(baseDistance + randomVariation, 248));
      
      const waterLevel = 250 - distance;
      
      let status;
      if (waterLevel < 150) status = "Aman";
      else if (waterLevel < 180) status = "Siaga";
      else status = "Bahaya";
      
      return {
        waktu: timeStr,
        tinggi: waterLevel,
        jarak: distance,
        status: status
      };
    }

    // Untuk koneksi serial nyata
    async function connectToArduino() {
      try {
        // Web Serial API (Chrome/Edge)
        const port = await navigator.serial.requestPort();
        await port.open({ baudRate: 9600 });
        
        const reader = port.readable.getReader();
        while (true) {
          const { value, done } = await reader.read();
          if (done) break;
          
          try {
            const text = new TextDecoder().decode(value);
            const data = JSON.parse(text);
            updateDisplay(data);
          } catch (e) {
            console.error("Error parsing data:", e);
          }
        }
      } catch (err) {
        console.log("Serial connection error:", err);
        // Fallback ke simulasi
        startSimulation();
      }
    }

    function startSimulation() {
      // Update pertama kali
      updateDisplay(simulateData());
      
      // Update setiap 10 detik
      setInterval(() => {
        updateDisplay(simulateData());
      }, 10000);
    }

    // Inisialisasi
    initChart();
    
    // Coba konek ke Arduino, jika tidak bisa gunakan simulasi
    if ('serial' in navigator) {
      connectToArduino();
    } else {
      document.getElementById('last-update').textContent += " (Mode Simulasi)";
      startSimulation();
    }

    // Indikator update real-time
    setInterval(() => {
      const secondsSinceUpdate = (Date.now() - lastUpdateTime) / 1000;
      if (secondsSinceUpdate > 15) {
        document.getElementById('last-update').style.color = 'red';
      } else {
        document.getElementById('last-update').style.color = '';
      }
    }, 1000);
  </script>
</body>
</html>