<?php
// --- CONFIG ---
$secret_key = "fremsoft";
$file_json  = "status_maedc.json";
$file_cmd   = "eroga_subito.json";
$file_csv   = "storico_dati.csv";

// --- CHECK PARAMS ---
if (!isset($_GET['key']) || $_GET['key'] !== $secret_key) {
    http_response_code(403);
    echo "Accesso negato.";
    exit;
}

// --- SE PRESENTE 'status', AGGIORNA FILE JSON ---
if (isset($_GET['status'])) {
	$status_base64 = $_GET['status'];

    // Decodifica la stringa Base64 
	// Vedi: https://www.base64encode.org/
    $status_json = base64_decode($status_base64);
    
    // Decodifica per validare che sia JSON valido
    $decoded = json_decode($status_json, true);
 
    if (!is_array($decoded)) {
        http_response_code(400);
        echo "Parametro 'status' non è un JSON valido.";
        exit;
    }

    // --- Campi obbligatori da controllare ---
    $campi_obbligatori = ['cibo', 'ultimo_pasto', 'stato', 'allarme'];

    foreach ($campi_obbligatori as $campo) {
        if (!array_key_exists($campo, $decoded)) {
            http_response_code(400);
            echo "Campo mancante nel JSON: '$campo'.";
            exit;
        }
    }

    // Scrive il file JSON (leggibile e UTF-8 safe)
	$decoded['timestamp'] = time(); 
	$status_json = json_encode($decoded, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
    file_put_contents($file_json, $status_json);

    //die( "OK: status aggiornato.\n" );
	
	// --- Verifica presenza comando erogasubito ---
	header('Content-Type: application/json');    
    if (file_exists($file_cmd)) {
        echo file_get_contents($file_cmd);
        exit;
    } else {
        $data = [ 'timestamp' => 0,	'quantita' => 0 ];
		$json_data = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
		echo $json_data;
        exit;
    }
}

if (isset($_GET['erogasubito'])) {
    $quantita = intval($_GET['erogasubito']);
    
    if (($quantita <= 0) || ($quantita > 500)) {
        http_response_code(400);
        die("Quantità non valida.");
    }

    $data = [
        'timestamp' => time(),
        'quantita' => $quantita
    ];

    $json_data = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);

    if (file_put_contents($file_cmd, $json_data)) {
        die("OK: comando registrato.");
    } else {
        http_response_code(500);
        die("Errore nel salvataggio.");
    }
} 

if (isset($_GET['cancella_erogasubito'])) {
    if (file_exists($file_cmd)) {
        unlink($file_cmd);
        die("OK: file comando cancellato.\n");
    } else {
        die("Nessun comando da cancellare.\n");
    }
}

if (!isset($_GET['quantita'])) {
    http_response_code(400);
    echo "Parametro 'quantita' mancante.";
    exit;
}

// --- PREPARA NUOVA RIGA ---
date_default_timezone_set('Europe/Rome');
$data_ora = date('d/m/Y H:i'); // Formato "27/04/2025 08:30"
$quantita = $_GET['quantita']; // Esempio "100,2"

// --- LEGGI TUTTO IL FILE ESISTENTE ---
if (!file_exists($file_csv)) {
    // Se il file non esiste ancora, crea l'intestazione
    $righe = ["data_ora,quantita"];
} else {
    $righe = file($file_csv, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
}

// --- AGGIUNGI LA NUOVA RIGA IN CIMA ---
array_splice($righe, 1, 0, "\"{$data_ora}\",\"{$quantita}\"");

// --- TAGLIO A 100 RIGHE MAX (1 intestazione + 100 righe dati) ---
if (count($righe) > 21) {
    $righe = array_slice($righe, 0, 21);
}

// --- SCRIVI TUTTO IL FILE NUOVO ---
file_put_contents($file_csv, implode("\n", $righe));

// --- RISPOSTA ---
echo "OK: aggiunto pasto $quantita g alle $data_ora";
?>