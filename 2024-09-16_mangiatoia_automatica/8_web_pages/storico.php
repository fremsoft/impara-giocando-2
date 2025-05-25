<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Storico Pasti - MAEDC</title>
  <style>
    body {
      margin: 0;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(to bottom right, #c2e9fb, #a1c4fd);
      display: flex;
      flex-direction: column;
      min-height: 100vh;
      padding: 20px;
    }

    header {
      text-align: center;
      margin-bottom: 20px;
    }

    header h1 {
      font-size: 2rem;
      color: #222;
    }

    .table-container {
      background: #fff;
      border-radius: 20px;
      padding: 20px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
      overflow-x: auto;
    }

    table {
      width: 100%;
      border-collapse: collapse;
    }

    th, td {
      padding: 12px;
      text-align: center;
      border-bottom: 1px solid #ddd;
      font-size: 1rem;
    }

    th {
      background-color: #007bff;
      color: white;
      font-weight: bold;
    }

    tr:hover {
      background-color: #f1f1f1;
    }

    a.button {
      display: inline-block;
      margin-top: 20px;
      padding: 10px 20px;
      font-size: 1rem;
      font-weight: bold;
      color: #fff;
      background-color: #28a745;
      border-radius: 10px;
      text-decoration: none;
      text-align: center;
    }

    a.button:hover {
      background-color: #218838;
    }

    footer {
      margin-top: auto;
      font-size: 0.8rem;
      color: #777;
      text-align: center;
      padding: 10px;
    }

    @media (max-width: 500px) {
      th, td {
        font-size: 0.9rem;
      }
    }
  </style>
</head>
<body>

<header>
  <h1>📜 Storico Pasti Serviti</h1>
</header>

<div class="table-container">
  <table id="storicoTable">
    <thead>
      <tr>
        <th>Data e ora</th>
        <th>Quantità (g)</th>
      </tr>
    </thead>
    <tbody>
      <!-- I dati saranno caricati qui dinamicamente -->
    </tbody>
  </table>
</div>

<a class="button" href="https://fremsoft.it/imparagiocando/maedc/index.php">⬅️ Torna al Monitoraggio</a>

<footer>
  &copy; 2025 Impara Giocando - Tutti i diritti riservati
</footer>

<script>

<?php
  /* const pasti = [
    { data_ora: '27/04/2025 08:30', quantita: 120 },
    { data_ora: '27/04/2025 20:30', quantita: 110 },
    { data_ora: '26/04/2025 08:30', quantita: 125 },
    { data_ora: '26/04/2025 20:30', quantita: 115 },
  ]; */
  $filename = 'storico_dati.csv';
  
  if (file_exists($filename)) {
	  
	print("const pasti = [");
	
    if (($handle = fopen($filename, 'r')) !== false) {
        // Salta l'intestazione
        fgetcsv($handle, length: 0, separator: ',', enclosure: '"', escape: '\\');

        // Legge riga per riga
        while (($row = fgetcsv($handle)) !== false) {
            // row[0] = data_ora, row[1] = quantita
			print ( "{ data_ora: '{$row[0]}', quantita: '{$row[1]}'}," );
        }
		
        fclose($handle);
    }
	
	print("];");
}
?>
 
  const tbody = document.getElementById('storicoTable').querySelector('tbody');

  pasti.forEach((pasto, index) => {
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${pasto.data_ora}</td><td>${pasto.quantita} g</td>`;
    tbody.appendChild(tr);
  });
  
  setTimeout(function() { window.location.reload(); }, 10000);
</script>

</body>
</html>
