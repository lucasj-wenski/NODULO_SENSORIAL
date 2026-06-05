var NOME_ARQUIVO = "Estufa_Tigas_V2"; // Nome novo para criar uma planilha zerada

// --- ROTEADOR PRINCIPAL ---
function doGet(e) {
  if (e.parameter && e.parameter.temp) {
    return salvarDados(e);
  }
  return HtmlService.createTemplateFromFile('Index')
      .evaluate()
      .setTitle('Dashboard Estufa Tigas')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL)
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
}

// --- FUNÇÃO 1: SALVA DADOS DO ESP32 NA PLANILHA ---
function salvarDados(e) {
  var sheet = getOrCreateSheet();
  
  // O SEGREDO DO GRÃO MESTRE: O Apóstrofo (') antes do parâmetro
  // Isso impede que o Google transforme 25.4 em 25 de Abril!
  var temp = e.parameter.temp ? "'" + e.parameter.temp : "'--";
  var umid = e.parameter.umid ? "'" + e.parameter.umid : "'--";
  
  var gas = e.parameter.gas || 0;
  var solo = e.parameter.solo || 0;
  var fita = e.parameter.fita || 0;
  var r1 = e.parameter.r1 || 0;
  var r2 = e.parameter.r2 || 0;
  var ip = e.parameter.esp_ip || "Aguardando..."; 
  var dataHora = new Date(); 
  
  // Salva na planilha
  sheet.appendRow([dataHora, temp, umid, gas, solo, fita, r1, r2, ip]);
  
  // Checa comandos
  var commandCell = sheet.getRange("J1");
  var command = commandCell.getValue();
  
  if (command != "") {
    commandCell.setValue(""); 
    return ContentService.createTextOutput(command);
  } else {
    return ContentService.createTextOutput("OK");
  }
}

// --- FUNÇÃO 2: MANDA DADOS PARA O SITE (DASHBOARD) ---
function getDadosAtualizados() {
  var sheet = getOrCreateSheet();
  var lastRow = sheet.getLastRow();
  
  if (lastRow < 2) return { temp: "--", umid: "--", gas: "--", solo: "--", fita: false, r1: false, r2: false, ip: "", status: false };

  var dados = sheet.getRange(lastRow, 1, 1, 9).getValues()[0];
  
  var dataObj = new Date(dados[0]);
  var horaFormatada = Utilities.formatDate(dataObj, Session.getScriptTimeZone(), "HH:mm:ss");

  var agora = new Date();
  var diferencaSegundos = (agora - dataObj) / 1000;
  var isOnline = diferencaSegundos < 120; 

  return {
    hora: horaFormatada,
    temp: dados[1], // Agora vai chegar limpinho como "25.4"
    umid: dados[2],
    gas: dados[3],
    solo: dados[4],
    fita: (dados[5] == 1 || dados[5] == "1" || dados[5] === true),
    r1: (dados[6] == 1 || dados[6] == "1" || dados[6] === true),
    r2: (dados[7] == 1 || dados[7] == "1" || dados[7] === true),
    ip: dados[8], 
    status: isOnline 
  };
}

// --- FUNÇÃO 3: RECEBE COMANDOS DO SITE ---
function setCommand(cmd) {
  var sheet = getOrCreateSheet();
  sheet.getRange("J1").setValue(cmd); 
  return "Comando Registrado: " + cmd;
}

// --- FUNÇÃO AUXILIAR: CRIA PLANILHA ---
function getOrCreateSheet() {
  var files = DriveApp.getFilesByName(NOME_ARQUIVO);
  var spreadsheet;
  if (files.hasNext()) {
    spreadsheet = SpreadsheetApp.openById(files.next().getId());
  } else {
    spreadsheet = SpreadsheetApp.create(NOME_ARQUIVO);
    spreadsheet.getSheets()[0].appendRow(["Data/Hora", "Temp", "Umid", "Gás", "Solo", "Fita", "Relé 1", "Relé 2", "IP Câmera", "CMD (J1)"]);
  }
  return spreadsheet.getSheets()[0];
}
