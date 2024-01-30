#include "wifi.h"
#include "WiFiEspAT.h"



void cliWifi(cli_args_t *args);




bool wifiInit(void)
{
  bool ret;

  ret = WiFi.init(1000000);
  logPrintf("Wifi\n");
  logPrintf(" .init()   - %s\n", ret ? "OK":"Fail");
  logPrintf(" .status() - %d\n", WiFi.status());
  logPrintf(" .firm     - %s\n", WiFi.firmwareVersion());  
  logPrintf(" .baud     - %d bps\n", WiFi.getBaud());  

  cliAdd("wifi", cliWifi);
  return ret;
}

const char *wifiGetEncryptionTypeName(int thisType) 
{
  const char *p_name = "";

  switch (thisType) 
  {
    case ENC_TYPE_WEP:
      p_name = "WEP";
      break;
    case ENC_TYPE_TKIP:
      p_name = "WPA";
      break;
    case ENC_TYPE_CCMP:
      p_name = "WPA2";
      break;
    case ENC_TYPE_NONE:
      p_name = "None";
      break;
    case ENC_TYPE_AUTO:
      p_name = "Auto";
      break;
  }

  return p_name;
}

const char *wifiGetStatusName(uint8_t status)
{
  const char *p_name = "";

  switch(status)
  {
    case WL_NO_MODULE:
      p_name = "WL_NO_MODULE";
      break;
    case WL_IDLE_STATUS:
      p_name = "WL_IDLE_STATUS";
      break;
    case WL_NO_SSID_AVAIL:
      p_name = "WL_NO_SSID_AVAIL";
      break;
    case WL_CONNECTED:
      p_name = "WL_CONNECTED";
      break;
    case WL_CONNECT_FAILED:
      p_name = "WL_CONNECT_FAILED";
      break;
    case WL_CONNECTION_LOST:
      p_name = "WL_CONNECTION_LOST";
      break;
    case WL_DISCONNECTED:
      p_name = "WL_DISCONNECTED";
      break;
    case WL_AP_LISTENING:
      p_name = "WL_AP_LISTENING";
      break;
    case WL_AP_CONNECTED:
      p_name = "WL_AP_CONNECTED";
      break;
    case WL_AP_FAILED:
      p_name = "WL_AP_FAILED";
      break;
    default:
      break;
  }
  return p_name;
}

void cliWifi(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    IPAddress ip = WiFi.localIP();

    cliPrintf("status : %s\n", wifiGetStatusName(WiFi.status()));
    cliPrintf("firm   : %s\n", WiFi.firmwareVersion());    
    cliPrintf("baud   : %d bps\n", WiFi.getBaud());  
    cliPrintf("IP     : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "scan"))
  {
    int num_ssid;
    
    num_ssid = WiFi.scanNetworks();
    if (num_ssid > 0) 
    {
      cliPrintf("num ssid : %d\n", num_ssid);
      for (int i = 0; i < num_ssid; i++) 
      {
        cliPrintf("%d) %-16s rssi: %3ddBm %s\n", 
          i, 
          WiFi.SSID(i), 
          WiFi.RSSI(i), 
          wifiGetEncryptionTypeName(WiFi.encryptionType(i)));
      }
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "connect"))
  {
    const char *p_ssid;
    const char *p_pass;
    int status = WL_IDLE_STATUS;

    p_ssid = args->getStr(1);
    p_pass = args->getStr(2);

    status = WiFi.begin(p_ssid, p_pass);
    cliPrintf("status   : %s\n", wifiGetStatusName(status));
    if (status == WL_CONNECTED)
    {
      IPAddress ip = WiFi.localIP();
      cliPrintf("Local IP : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);      
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "web-server"))
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      cliPrintf("Not Connected");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      IPAddress ip = WiFi.localIP();
      WiFiServer server(80);

      server.begin();
      cliPrintf("Local IP : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

      while(cliKeepLoop())
      {
        WiFiClient client = server.available();

        if (client) 
        {
          cliPrintf("new client\n"); 
          String currentLine = "";      
          while (client.connected() && cliKeepLoop())
          {                            
            if (client.available())
            {                           
              char c = client.read();   
              cliPutch(c);
              if (c == '\n')
              {                         

                // if the current line is blank, you got two newline characters in a row.
                // that's the end of the client HTTP request, so send a response:
                if (currentLine.length() == 0)
                {
                  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                  // and a content-type so the client knows what's coming, then a blank line:
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-type:text/html");
                  client.println();

                  // the content of the HTTP response follows the header:
                  client.print("Click <a href=\"/H\">here</a> turn the LED ON<br>");
                  client.print("Click <a href=\"/L\">here</a> turn the LED OFF<br>");

                  // The HTTP response ends with another blank line:
                  client.println();
                  // break out of the while loop:
                  break;
                }
                else
                {                 // if you got a newline, then clear currentLine:
                  currentLine = "";
                }
              }
              else if (c != '\r')
              {                   // if you got anything else but a carriage return character,
                currentLine += c; // add it to the end of the currentLine
              }

              // Check to see if the client request was "GET /H" or "GET /L":
              if (currentLine.endsWith("GET /H"))
              {              
                ledOn(_DEF_LED1);
              }
              if (currentLine.endsWith("GET /L"))
              {
                ledOff(_DEF_LED1);
              }
            }
          }
          // close the connection:
          client.stop();
          cliPrintf("client disconnected\n");          
        }
      }
      server.end();
    }
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("wifi info\n");
    cliPrintf("wifi scan\n");
    cliPrintf("wifi connect ssid pass\n");
    cliPrintf("wifi web-server\n");
  }
}
