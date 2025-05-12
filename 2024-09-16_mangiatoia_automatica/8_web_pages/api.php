<?php
// --- CONFIG ---
$secret_key = "fremsoft";
$file_csv   = "storico_dati.csv";
$file_json  = "status_maedc.json";

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
    file_put_contents($file_json, $status_json);

    die( "OK: status aggiornato.\n" );
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