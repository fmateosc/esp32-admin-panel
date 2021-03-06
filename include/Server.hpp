/* Instancia de AsyncWebServer */
AsyncWebServer server(80);

// Cargar Información de las paginas al Servidor --------------------------------------
void InitServer(){
    /**********************************************/
    server.serveStatic("/bootstrap-responsive.min.css", SPIFFS, "/bootstrap-responsive.min.css").setDefaultFile("bootstrap-responsive.min.css");
    server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css").setDefaultFile("bootstrap.min.css");
    server.serveStatic("/styles.css", SPIFFS, "/styles.css").setDefaultFile("styles.css");
    server.serveStatic("/jquery.easy-pie-chart.css", SPIFFS, "/jquery.easy-pie-chart.css").setDefaultFile("jquery.easy-pie-chart.css");
    server.serveStatic("/bootstrap.min.js", SPIFFS, "/bootstrap.min.js").setDefaultFile("bootstrap.min.js");
    server.serveStatic("/jquery-1.9.1.min.js", SPIFFS, "/jquery-1.9.1.min.js").setDefaultFile("jquery-1.9.1.min.js");
    server.serveStatic("/jquery.easy-pie-chart.js", SPIFFS, "/jquery.easy-pie-chart.js").setDefaultFile("jquery.easy-pie-chart.js");
    server.serveStatic("/modernizr.min.js", SPIFFS, "/modernizr.min.js").setDefaultFile("modernizr.min.js");
    server.serveStatic("/sweetalert2.min.css", SPIFFS, "/sweetalert2.min.css").setDefaultFile("sweetalert2.min.css");
    server.serveStatic("/sweetalert2.min.js", SPIFFS, "/sweetalert2.min.js").setDefaultFile("sweetalert2.min.js");
    server.serveStatic("/scripts.js", SPIFFS, "/scripts.js").setDefaultFile("scripts.js");
    server.serveStatic("/glyphicons-halflings.png", SPIFFS, "/glyphicons-halflings.png").setDefaultFile("glyphicons-halflings.png");
    server.serveStatic("/glyphicons-halflings-white.png", SPIFFS, "/glyphicons-halflings-white.png").setDefaultFile("glyphicons-halflings-white.png");
    server.serveStatic("/logo.png", SPIFFS, "/logo.png").setDefaultFile("logo.png");
    
    /*********************INDEX.HTML*************************/
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      // Index.html
      File file = SPIFFS.open(F("/index.html"), "r");
      
      if (file){
        file.setTimeout(100);

        String s = file.readString();

        file.close();

        // Actualiza contenido dinamico del html
        s.replace(F("#id#"), id);
        s.replace(F("#serie#"), device_id);

        /* Bloque WIFI */
        s.replace(F("#WFEstatus#"), WiFi.status() == WL_CONNECTED ? F("<span class='label label-success'>CONECTADO</span>") : F("<span class='label label-important'>DESCONECTADO</span>"));
        s.replace(F("#WFSSID#"), WiFi.status() == WL_CONNECTED ? F(ssid) : F("--"));
        s.replace(F("#sysIP#"), ipStr(WiFi.status() == WL_CONNECTED ? WiFi.localIP() : WiFi.softAPIP()));
        s.replace(F("#WFDBM#"), WiFi.status() == WL_CONNECTED ? String(WiFi.RSSI()) : F("0"));
        s.replace(F("#WFPRC#"), WiFi.status() == WL_CONNECTED ? String(round(1.88 * (WiFi.RSSI() + 100)), 0) : F("0"));
        
        /* Bloque pie chart */
        s.replace(F("#SWFI#"), WiFi.status() == WL_CONNECTED ? String(round(1.88 * (WiFi.RSSI() + 100)), 0) : F("0"));
        s.replace(F("#PMEM#"), String(round(SPIFFS.usedBytes() * 100 / SPIFFS.totalBytes()), 0));
        s.replace(F("#ram#"), String(ESP.getFreeHeap() * 100 / ESP.getHeapSize()));
        s.replace(F("#cpu#"), String(TempCPU));

        // Envia dados al navegador
        request->send(200, "text/html", s);  
      }else{
        request->send(500, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
          "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
            "Swal.fire({title: 'Error!',"
              " text: 'Error 500 Internal Server Error',"
              " icon: 'error',"
              " confirmButtonText: 'Cerrar'}).then((result) => {"
              "if (result.isConfirmed){"
                    "window.location = '/';"
                "};"
            "})"
          "</script><body></html>");

        Serial.println(F("\nError: Config - ERROR leyendo el archivo"));  
      }
    });
    
    /*********************CONFIG WIFI HTML*************************/
    server.on("/configwifi", HTTP_GET, [](AsyncWebServerRequest *request){
        // Config
        File file = SPIFFS.open(F("/configwifi.html"), "r");
        if (file){
          file.setTimeout(100);
          String s = file.readString();
          file.close();
          // Atualiza el contenido al cargar
          s.replace(F("#id#"), id);
          s.replace(F("#ssid#"), ssid);
          //sección ap
          s.replace(F("#nameap#"), String(nameap));
          // Send data
          request->send(200, "text/html", s);
        }else{
          request->send(500, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'Error 500 Internal Server Error',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "window.location = '/';"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
          log(F("\nError: Config - ERROR leyendo el archivo"));
        }
    });
    /*********************SAVE CONFIG WIFI*************************/
    server.on("/confwifiSave", HTTP_POST, [](AsyncWebServerRequest *request){
        String response;
        StaticJsonDocument<300> doc;
        // Graba Configuración desde Config
        // Verifica número de campos recebidos
        // ESP32 envia 5 campos
        if (request->params() == 5){
          String s;
          // ID
          if(request->hasArg("id"))
          s = request->arg("id");
          s.trim();
          if (s == ""){
            s = deviceID();
          }
          strlcpy(id, s.c_str(), sizeof(id));
          // SSID
          if(request->hasArg("ssid"))
          s = request->arg("ssid");
          s.trim();
          if (s == ""){
            s = ssid;
          }
          strlcpy(ssid, s.c_str(), sizeof(ssid));
          // PW SSID
          if(request->hasArg("pw"))
          s = request->arg("pw");
          s.trim();
          if (s != ""){
            // Actualiza contraseña
            strlcpy(pw, s.c_str(), sizeof(pw));
          }
          // Nombre AP
          if(request->hasArg("nameap"))
          s = request->arg("nameap");
          s.trim();
          if (s != ""){
            // Atualiza ssid ap
            strlcpy(nameap, s.c_str(), sizeof(nameap));
          }
          // Contraseña AP
          if(request->hasArg("passwordap"))
          s = request->arg("passwordap");
          s.trim();
          if (s != ""){
            // Atualiza contraseña ap
            strlcpy(passwordap, s.c_str(), sizeof(passwordap));
          }
          // Graba configuracion
          if (configSave()){
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                            "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Hecho!',"
                                                                " text: 'Configuración WIFI guardada, se requiere reiniciar el Equipo',"
                                                                " icon: 'success',"
                                                                " showCancelButton: true,"
                                                                " confirmButtonColor: '#3085d6',"
                                                                " cancelButtonColor: '#d33',"
                                                                " confirmButtonText: 'Si, reiniciar',"
                                                                " cancelButtonText: 'Cancelar',"
                                                                " reverseButtons: true"
                                                                " }).then((result) => {"
                                                                              "if (result.isConfirmed){"
                                                                                  "window.location = 'reboot';"
                                                                              "}else if ("
                                                                                  "result.dismiss === Swal.DismissReason.cancel"
                                                                                "){"
                                                                                  "history.back();"
                                                                                "}"
                                                                          "})"
                                                  "</script><body></html>");
          }else{
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'No se pudo guardar, Falló la configuración WIFI',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
            log(F("\nError: ConfigSave - ERROR salvando Configuración"));
          }
        }else{
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'No se pudo guardar, Error de parámetros WIFI',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
        }
    });
    /**********************CONFIG SENSORES HTML************************/
    server.on("/configsensores", HTTP_GET, [](AsyncWebServerRequest *request){
    // Config
    File file = SPIFFS.open(F("/configsensores.html"), "r");

    if (file){
      file.setTimeout(100);

      String s = file.readString();

      file.close();
      
      // Atualiza el contenido al cargar
      s.replace(F("#temp1#"), String(umbralTemp1));
      s.replace(F("#temp2#"), String(umbralTemp2));
      s.replace(F("#temp3#"), String(umbralTemp3));
      s.replace(F("#temp4#"), String(umbralTemp4));
      
      // Send data
      request->send(200, "text/html", s);
    }else{
      request->send(500, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
        "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
          "Swal.fire({title: 'Error!',"
          " text: 'Error 500 Internal Server Error',"
          " icon: 'error',"
          " confirmButtonText: 'Cerrar'}).then((result) => {"
            "if (result.isConfirmed){"
                "window.location = '/';"
            "};"
          "})"
        "</script><body></html>");

      Serial.print(F("\nError: Config - ERROR leyendo el archivo"));
    }
    });

    /************************SAVE CONFIG SENSORES**********************/
    server.on("/confsensoresSave", HTTP_POST, [](AsyncWebServerRequest *request){
      String response;

      StaticJsonDocument<300> doc;

      // Graba Configuración desde Config
      // Verifica número de campos recebidos
      // ESP32 envia 4 campos
      if (request->params() == 4){
        String s;     

        // Temp1
        if(request->hasArg("temp1"))
        s = request->arg("temp1");
        s.trim();

        if (s == ""){
          s = umbralTemp1;
        }

        umbralTemp1 = s.toInt();
        
        // Temp2
        if(request->hasArg("temp2"))
        s = request->arg("temp2");
        s.trim();

        if (s == ""){
          s = umbralTemp2;
        }

        umbralTemp2 = s.toInt();

        // Temp3
        if(request->hasArg("temp3"))
        s = request->arg("temp3");
        s.trim();

        if (s == ""){
          s = umbralTemp3;
        }

        umbralTemp3 = s.toInt();

        // Temp4
        if(request->hasArg("temp4"))
        s = request->arg("temp4");
        s.trim();

        if (s == ""){
          s = umbralTemp4;
        }

        umbralTemp4 = s.toInt();
        
        // Graba configuracion
        if (configSave()){
          request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
            "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
              "Swal.fire({title: 'Hecho!',"
              " text: 'Configuración sensores guardada, se requiere reiniciar el Equipo',"
              " icon: 'success',"
              " showCancelButton: true,"
              " confirmButtonColor: '#3085d6',"
              " cancelButtonColor: '#d33',"
              " confirmButtonText: 'Si, reiniciar',"
              " cancelButtonText: 'Cancelar',"
              " reverseButtons: true"
              " }).then((result) => {"
                  "if (result.isConfirmed){"
                      "window.location = 'reboot';"
                  "}else if ("
                      "result.dismiss === Swal.DismissReason.cancel"
                    "){"
                      "history.back();"
                    "}"
                "})"
            "</script><body></html>");
        }else{
          request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
            "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
              "Swal.fire({title: 'Error!',"
              " text: 'No se pudo guardar, Falló la configuración de los sensores',"
              " icon: 'error',"
              " confirmButtonText: 'Cerrar'}).then((result) => {"
                  "if (result.isConfirmed){"
                      "history.back();"
                  "};"
                "})"
            "</script><body></html>");

          Serial.println(F("\nError: ConfigSave - ERROR salvando Configuración"));
        }
      }else{
        request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
          "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
            "Swal.fire({title: 'Error!',"
            " text: 'No se pudo guardar, Error de parámetros de los sensores',"
            " icon: 'error',"
            " confirmButtonText: 'Cerrar'}).then((result) => {"
                "if (result.isConfirmed){"
                    "history.back();"
                "};"
              "})"
          "</script><body></html>");
      }
    });

    /*********************CONFIG WATSAPP HTML*************************/
    server.on("/configwhatsapp", HTTP_GET, [](AsyncWebServerRequest *request){
      // Config
      File file = SPIFFS.open(F("/configwhatsapp.html"), "r");

      if (file){
        file.setTimeout(100);

        String s = file.readString();

        file.close();

        // Atualiza el contenido al cargar
        s.replace(F("#apiKey#"), apiKey);
        s.replace(F("#numTelefono#"), phoneNumber);

        request->send(200, "text/html", s);
      }else{
        request->send(500, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                    "Swal.fire({title: 'Error!',"
                                                              " text: 'Error 500 Internal Server Error',"
                                                              " icon: 'error',"
                                                              " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                  "if (result.isConfirmed){"
                                                                                                      "window.location = '/';"
                                                                                                  "};"
                                                                                              "})"
                                                "</script><body></html>");
        log(F("\nError: Config - ERROR leyendo el archivo"));
      }
    });
    /*********************SAVE CONFIG WATSAPP*************************/
    server.on("/confWatsAppSave", HTTP_POST, [](AsyncWebServerRequest *request){
        String response;
        StaticJsonDocument<300> doc;
        // Graba Configuración desde Config
        // Verifica número de campos recebidos
        // ESP32 envia 2 campos
        if (request->params() == 2){
          String s;
          // Api Key
          if(request->hasArg("apiKey"))
          s = request->arg("apiKey");
          s.trim();

          if (s == ""){
            s = apiKey;
          }

          strlcpy(apiKey, s.c_str(), sizeof(apiKey));

          // Nº teléfono
          if(request->hasArg("numTelefono"))
          s = request->arg("numTelefono");
          s.trim();

          if (s == ""){
            s = phoneNumber;
          }

          strlcpy(phoneNumber, s.c_str(), sizeof(phoneNumber));
          
          // Graba configuracion
          if (configSave()){
            request->send(200, "text/html", 
              "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                            "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Hecho!',"
                                                                " text: 'Configuración WatsApp guardada, se requiere reiniciar el Equipo',"
                                                                " icon: 'success',"
                                                                " showCancelButton: true,"
                                                                " confirmButtonColor: '#3085d6',"
                                                                " cancelButtonColor: '#d33',"
                                                                " confirmButtonText: 'Si, reiniciar',"
                                                                " cancelButtonText: 'Cancelar',"
                                                                " reverseButtons: true"
                                                                " }).then((result) => {"
                                                                              "if (result.isConfirmed){"
                                                                                  "window.location = 'reboot';"
                                                                              "}else if ("
                                                                                  "result.dismiss === Swal.DismissReason.cancel"
                                                                                "){"
                                                                                  "history.back();"
                                                                                "}"
                                                                          "})"
                                                  "</script><body></html>");
          }else{
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'No se pudo guardar, Falló la configuración WIFI',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
            log(F("\nError: ConfigSave - ERROR salvando Configuración"));
          }
        }else{
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'No se pudo guardar, Error de parámetros WIFI',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
        }
    });

    /**************************ACTUALIZA LAS TEMPERATURAS***********************************/
    server.on("/getTemperatures", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", readTemperature());
    });
    /****************************************************************************************/

    /**********************************************/   
    server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(404, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error 404!',"
                                                                " text: 'Página no encontrada',"
                                                                " icon: 'warning',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
    });
    /**********************************************/
	  server.begin();
    log("\nInfo: HTTP server iniciado");
    /**********************************************/
    

}