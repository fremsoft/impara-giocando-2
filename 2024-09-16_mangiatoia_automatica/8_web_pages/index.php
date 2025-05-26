<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>#MAEDC - Stato Mangiatoia</title>
  <style>
    /* css */
    body {
      margin: 0;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(to bottom right, #a1c4fd, #c2e9fb);
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      padding: 20px;
    }

    header {
      text-align: center;
      margin-bottom: 20px;
    }

    header h1 {
      font-size: 2.5rem;
      color: #222;
      margin: 0;
    }

    .image-container {
	  width: 100%;
      max-width: 400px;
      margin: 20px 0;
    }

    .image-container img {
      background: white;
      width: 100%;
      height: auto;
      border-radius: 20px;
      box-shadow: 0 4px 10px rgba(0,0,0,0.2);
    }

    .status-container {
      background: #fff;
      border-radius: 20px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
      padding: 25px;
      width: 100%;
      max-width: 400px;
      text-align: center;
      margin-bottom: 20px;
    }

    .status-item {
      margin: 15px 0;
      font-size: 1.2rem;
      color: #333;
    }

    .status-label {
      font-weight: bold;
      color: #555;
    }

    .ok {
      color: green;
      font-weight: bold;
    }

    .alarm {
      color: red;
      font-weight: bold;
    }

    .actions-container {
      background: #fff;
      border-radius: 20px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
      padding: 20px;
      width: 100%;
      max-width: 400px;
      text-align: center;
    }

    .button {
      display: inline-block;
      margin: 10px;
      padding: 10px 20px;
      font-size: 1rem;
      font-weight: bold;
      color: #fff;
      background-color: #007bff;
      border: none;
      border-radius: 10px;
      text-decoration: none;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .button:hover {
      background-color: #0056b3;
    }

    input[type="number"] {
      padding: 10px;
      width: 80px;
      border: 1px solid #ccc;
      border-radius: 10px;
      margin-right: 10px;
      font-size: 1rem;
    }

    footer {
      margin-top: auto;
      font-size: 0.8rem;
      color: #777;
      text-align: center;
      padding: 10px;
    }

    /* Responsive tweak */
    @media (max-width: 500px) {
      .status-item, .button {
        font-size: 1rem;
      }
    }
  </style>
</head>
<body>

<header>
  <h1>MAEDC - Stato Mangiatoia</h1>
</header>

<div class="image-container">
  <img src="https://fremsoft.it/imparagiocando/maedc/images/maedc_300x410.png" alt="Immagine della Mangiatoia">
</div>

<?php

/* Guarda la lezione integrale: https://youtube.com/live/F3zWKNczNNs */
$file_json  = "status_maedc.json";

$status_json = file_get_contents($file_json);
$status_data = json_decode($status_json, true);

// Verifica se il JSON è valido
if (json_last_error() !== JSON_ERROR_NONE) {
    die("Errore nel decodificare il JSON.");
}

// Estrai i dati dal JSON
$presenza_cibo = ($status_data['cibo']==1)?("Presente"):("Assente");
$quantita_ultimo_pasto = ($status_data['ultimo_pasto']['quantita'])."&nbsp;g";
$data_ora_ultimo_pasto = ($status_data['ultimo_pasto']['data_ora']);
$stato = ($status_data['stato']=="OK")?("<span class='ok'>OK</span>"):("<span class='alarm'>".$status_data['stato']."</span>");
$allarme = ((!empty($status_data['allarme']))&&($status_data['allarme']!=="Nessun allarme"))
           ?("<div class='status-item alarm'>⚠️ Allarme:".$status_data['allarme']."</div>")
           :("");
		   
// Calcolo stato connessione basato sul timestamp del file
$last_update = filemtime($file_json);
$now = time();
$seconds_since_update = $now - $last_update;

// "Segnale di vita" almeno ogni 15 minuti
$connessione = ($seconds_since_update <= 60*15) 
    ? "<span class='ok'>Online</span>"
    : "<span class='alarm'>Offline</span>";
$connessione .= " <small>(".$seconds_since_update."s fa)</small>";
?>

<div id="status-container" class="status-container">
  <div class="status-item">
    <span class="status-label">Presenza Cibo:</span> <span id="presenza_cibo"><?php echo $presenza_cibo; ?></span>
  </div>
  <div class="status-item">
    <span class="status-label">Quantità Ultimo Pasto:</span> <span id="quantita_ultimo_pasto"><?php echo $quantita_ultimo_pasto; ?></span>
  </div>
  <div class="status-item">
    <span class="status-label">Data/Ora:</span> <span id="data_ora_ultimo_pasto"><?php echo $data_ora_ultimo_pasto; ?></span>
  </div>
  <div class="status-item">
    <span class="status-label">Connessione:</span> <span id="connessione"><?php echo $connessione; ?></span>
  </div>
  <div class="status-item">
    <span class="status-label">Stato:</span> <span id="stato"><?php echo $stato; ?></span>
  </div>
  <!-- Se allarme attivo -->
   <div id="allarme"><?php echo $allarme; ?></div>
</div>

<div class="actions-container">
  <a class="button" href="https://fremsoft.it/imparagiocando/maedc/storico.php">📜 Storico Pasti</a>
  
  <div style="margin-top: 20px;">
    <div><input type="number" id="quantita" min="0" max="500" step="10" value="0"> grammi</div>
    <button class="button" onclick="erogaCibo()">Eroga subito</button>
  </div>
</div>

<footer>
  &copy; 2025 Impara Giocando - Tutti i diritti riservati
  
  <a href="https://youtube.com/live/RrfQHqaGorw">Guarda la lezione integrale</a>
</footer>

<script>
  function aggiornaStatus() {
    fetch('status_maedc.json?t=' + new Date().getTime())
	.then(response => {
      if (!response.ok) throw new Error("Errore nel recupero dello status");
      return response.json();
    })
    .then(data => {
      // Aggiorna ciascun campo
      document.getElementById("presenza_cibo").textContent = data.cibo ? "Presente" : "Assente";
      document.getElementById("quantita_ultimo_pasto").textContent = data.ultimo_pasto.quantita + "g";
      document.getElementById("data_ora_ultimo_pasto").textContent = data.ultimo_pasto.data_ora;
	  
      document.getElementById("stato").innerHTML = (data.stato == "OK")?"<span class='ok'>OK</span>":"<span class='error'>"+data.stato+"</span>";
	  
	  let allarme = "";
	  if (data.allarme && data.allarme !== "Nessun allarme") {
		  allarme = "<div class='status-item alarm'>⚠️ Allarme: " + data.allarme + "</div>";
	  }
      document.getElementById("allarme").innerHTML = allarme;
	  
      // Calcolo tempo trascorso
      const timestamp = data.timestamp;
      const now = Math.floor(Date.now() / 1000);
      const diff = now - timestamp;

      let conn = diff > (60*15) ? "<span class='alarm'>Offline</span>" : "<span class='ok'>Online</span>";
      conn += ` <small>(${diff}s fa)</small>`;
      document.getElementById("connessione").innerHTML = conn;
    })
    .catch(error => {
      console.error("Errore nel caricamento dello status:", error);
    });
  }

  function erogaCibo() {
	  
	const quantita = parseInt(document.getElementById('quantita').value);
    if (!quantita || quantita <= 0) {
      alert('Inserisci una quantità valida!');
      return;
    }

	const key = 'fremsoft';
	fetch(`api.php?key=${key}&erogasubito=${quantita}`)
      .then(response => response.text())
      .then(data => { alert('Comando inviato: ' + data); })
      .catch(error => {
        console.error('Errore nella richiesta:', error);
        alert('Errore durante la richiesta.');
      });
	  
  }
  
  setInterval( aggiornaStatus, 10000 );
  /* setTimeout( function() { window.location.reload(); }, 10000); */
    
</script>

</body>
</html>
