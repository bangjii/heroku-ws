/*
  * Author : bangjii
  * File Name : index.js
  * Function : WebSocket Server
  * Detail : Handle client message to private and broadcast
*/
const express = require('express');  
const SocketServer = require('ws').Server;
const path = require('path');
const PORT = process.env.PORT || 2222;
const INDEX = path.join(__dirname, 'index.html');

const server = express()
  .use((req, res) => res.sendFile(INDEX) )
  .listen(PORT, () => console.log('Listening on ' + PORT));

const wss = new SocketServer({ server });

var userKey = {}; // buat kumpulin id client

// function Broadcast to all.
wss.broadcast = function broadcast(data) {
	wss.clients.forEach(function each(client) {
		client.send(data);
	});
};

//ping client to keepalive connection
setInterval(() => {
  wss.broadcast("PING!");
}, 1000);

//if connection happen	
wss.on('connection', function (ws, req) {
	//user validation on first connection {user: "jamblang"}
	try {
		//ambil param json (user) diakhir url (setelah '/')
		var data = JSON.parse(decodeURIComponent(req.url.substring(req.url.lastIndexOf('/') + 1)));
	}
	catch(e) {
		console.log('connection closed due to invalid JSON input data');
		ws.close();
		return;
	}
  
	var subs = decodeURIComponent(req.url);	
	var indexK = subs.lastIndexOf('/');
	var jsonID = subs.substring(indexK + 1);  
	var objID = JSON.parse(jsonID);
	var valID = objID.user;
	/*
		Client send : {to:"user1",textMsg:"hola"}
		Client received : {tipe: "private,broad,notif", from:"user2",textMsg:"hola"}
	*/
	userKey[valID] = ws;
	console.log('connected: ' + valID + ' in ' + Object.getOwnPropertyNames(userKey));
	//send to debug page
	if (userKey['debug']){
		userKey['debug'].send('connected: ' + valID + ' in ' + Object.getOwnPropertyNames(userKey));
	}
	ws.on('message', function (message) {
		try {
			//pancing beneran json apa kagak
			var data = JSON.parse(message);
		}
		catch(e) {
			console.log('connection closed due to invalid JSON input data');
			ws.close();
			return;
		}
    
		console.log('received from ' + valID + ': ' + message);
		var messageArray = JSON.parse(message)
		var toID = messageArray.to;
		var toMsg = messageArray.textMsg;
		var toUserWebSocket = userKey[toID];
		var fromUserWebSocket = userKey[valID];
		var reply = {"tipe": "private", "from": valID, "textMsg": toMsg};
		var brd = {"tipe": "broadcast", "from": valID, "textMsg": toMsg};
		var offline = {"tipe": "notification", "from": "Server", "textMsg": toID + " is offline"};
		//send to debug page
		if (userKey['debug']){
			userKey['debug'].send('received from ' + valID + ': ' + message);
		}
		//private message
		if (toUserWebSocket) {
			console.log('sent to ' + toID + ': ' + toMsg);			
			toUserWebSocket.send(JSON.stringify(reply));
		}
		
		//if user offline (not in list)
		else if(!toUserWebSocket && toID != "brdALL"){
			console.log(toID + ' is offline');			
			fromUserWebSocket.send(JSON.stringify(offline));
		}
		
		//if message broadcast
		if (toID == "brdALL"){
			wss.broadcast(JSON.stringify(brd));
			console.log('broadcast: %s', JSON.stringify(brd));
		}
	});
	ws.on('close', function () {
		//remove user when disconnected
		delete userKey[valID]
		console.log('deleted: ' + valID)
	});
});
