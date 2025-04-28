<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>MAEDC - Stato Mangiatoia</title>
  <style>
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
  <img src="https://fremsoft.it/imparagiocando/images/maedc_300x410.png" alt="Immagine della Mangiatoia">
</div>

<div class="status-container">
  <div class="status-item">
    <span class="status-label">Presenza Cibo:</span> Presente
  </div>
  <div class="status-item">
    <span class="status-label">Quantità Ultimo Pasto:</span> 120g
  </div>
  <div class="status-item">
    <span class="status-label">Connessione:</span> <span class="ok">Online</span>
  </div>
  <div class="status-item">
    <span class="status-label">Stato:</span> <span class="ok">OK</span>
  </div>
  <!-- Se allarme attivo -->
  <div class="status-item alarm">
    ⚠️ Allarme: Cibo esaurito
  </div>
</div>

<div class="actions-container">
  <a class="button" href="https://fremsoft.it/imparagiocando/storico.php">📜 Storico Pasti</a>
  
  <div style="margin-top: 20px;">
    <div><input type="number" id="quantita" min="0" max="500" step="10" value="0"> grammi</div>
    <button class="button" onclick="erogaCibo()">Eroga subito</button>
  </div>
</div>

<footer>
  &copy; 2025 Impara Giocando - Tutti i diritti riservati
</footer>

<script>
  function erogaCibo() {
    const quantita = document.getElementById('quantita').value;
    if (!quantita || quantita <= 0) {
      alert('Inserisci una quantità valida!');
      return;
    }

    // Esempio POST finto (da adattare)
    /*
    fetch('/eroga', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ quantita: quantita })
    }).then(response => {
      if (response.ok) {
        alert('Erogazione avviata con successo!');
      } else {
        alert('Errore durante l\'erogazione!');
      }
    });
    */
  }
  
  setTimeout(function() { window.location.reload(); }, 10000);
</script>

</body>
</html>
