<?php
// --- CONFIG ---
$secret_key = "fremsoft";
$file_csv = "storico_dati.csv";

// --- CHECK PARAMS ---
if (!isset($_GET['key']) || $_GET['key'] !== $secret_key) {
    http_response_code(403);
    echo "Accesso negato.";
    exit;
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

// --- SCRIVI TUTTO IL FILE NUOVO ---
file_put_contents($file_csv, implode("\n", $righe));

// --- RISPOSTA ---
echo "OK: aggiunto pasto $quantita g alle $data_ora";
?>