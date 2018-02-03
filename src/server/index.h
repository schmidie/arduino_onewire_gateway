#include <pgmspace.h>
#include <Arduino.h>

// save index page on flash-memory (NOT SRAM or EEPROM)!
static const char* INDEX_PAGE PROGMEM = R"=====(
  <HTML>
  	<HEAD>
  			<TITLE>energeer.de</TITLE>
        <style type="text/css">
      .form-style-6{
          font: 95% Arial, Helvetica, sans-serif;
          max-width: 400px;
          margin: 10px auto;
          padding: 16px;
          background: #F3F2F1;
      }
      .form-style-6 h1{
          background: #5BC4B6;
          padding: 20px 0;
          font-size: 140%;
          font-weight: 300;
          text-align: center;
          color: #fff;
          margin: -16px -16px 16px -16px;
      }
      .form-style-6 input[type="text"],
      .form-style-6 input[type="date"],
      .form-style-6 input[type="datetime"],
      .form-style-6 input[type="email"],
      .form-style-6 input[type="number"],
      .form-style-6 input[type="search"],
      .form-style-6 input[type="time"],
      .form-style-6 input[type="url"],
      .form-style-6 input[type="password"],
      .form-style-6 textarea,
      .form-style-6 select
      {
          -webkit-transition: all 0.30s ease-in-out;
          -moz-transition: all 0.30s ease-in-out;
          -ms-transition: all 0.30s ease-in-out;
          -o-transition: all 0.30s ease-in-out;
          outline: none;
          box-sizing: border-box;
          -webkit-box-sizing: border-box;
          -moz-box-sizing: border-box;
          width: 100%;
          background: #fff;
          margin-bottom: 4%;
          border: 1px solid #ccc;
          padding: 3%;
          color: #555;
          font: 95% Arial, Helvetica, sans-serif;
      }
      .form-style-6 input[type="text"]:focus,
      .form-style-6 input[type="date"]:focus,
      .form-style-6 input[type="datetime"]:focus,
      .form-style-6 input[type="email"]:focus,
      .form-style-6 input[type="number"]:focus,
      .form-style-6 input[type="search"]:focus,
      .form-style-6 input[type="time"]:focus,
      .form-style-6 input[type="url"]:focus,
      .form-style-6 input[type="password"]:focus,
      .form-style-6 textarea:focus,
      .form-style-6 select:focus
      {
          box-shadow: 0 0 5px #F0A145;
          padding: 3%;
          border: 1px solid #F0A145;
      }

      .form-style-6 input[type="submit"],
      .form-style-6 input[type="button"]{
          box-sizing: border-box;
          -webkit-box-sizing: border-box;
          -moz-box-sizing: border-box;
          width: 100%;
          padding: 3%;
          background: #5BC4B6;
          border-bottom: 2px solid #30C29E;
          border-top-style: none;
          border-right-style: none;
          border-left-style: none;
          color: #fff;
      }
      .form-style-6 input[type="submit"]:hover,
      .form-style-6 input[type="button"]:hover{
          background: #53B3A6;
      }
      </style>
  	</HEAD>
  <BODY>
    <div class="form-style-6">
      <h1>energeer.de WLAN-Einstellungen</h1>
      <form action='/settings'>
        <input type="text" name="ssid" placeholder="SSID" />
        <input type="password" name="password" placeholder="Passwort" />
        <input type="submit" value="speichern" />
      </form>
    </div>
  </BODY>
  </HTML>
)=====";
