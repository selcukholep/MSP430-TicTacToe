let net = require('net');

let rooms = new Map();

let TCP_STARTER_DATA_PREFIX   =   "S";
let TCP_WAITER_DATA_PREFIX    =   "R";

let TCP_GAME_DATA_PREFIX_BY_STATUS = [ "C", "W", "L", "D" ];

let TCP_ERROR_CODE_PAIR_CONNECTION_LOST = "E0";
let TCP_ERROR_CODE_PAIR_HAS_LEFT        = "E1";

let ROOM_INITIALIZED                = 0;
let ROOM_GAME_STARTED               = 1;

let CLIENT_JOINED                   = 0;
let CLIENT_USERNAME_TAKEN           = 1;
let CLIENT_PAIR_USERNAME_SENT       = 2;
let CLIENT_START_INFO_SENT          = 3;
let CLIENT_READY                    = 10; // Current player
let CLIENT_BUSY                     = 20; // Current waiter

net.createServer(function (socket) {

    let clientId = socket.remoteAddress + ":" + socket.remotePort;
    let roomId = getAvailableRoomIndex(clientId);
    let room = rooms.get(roomId);

    room.clients.set(clientId, {
        username: null,
        socket: socket,
        status: CLIENT_JOINED,
        score: 0,
        symbol: room.clients.size > 0 ? room.clients.get(room.clients.keys().next().value).symbol ^ 1
            : room.whoseTurnIndex,
        pairId: room.clients.size > 0 ? room.clients.keys().next().value : null,
        id: clientId
    });

    if (room.clients.size === 2) {
        room.whoseTurn = room.whoseTurnIndex === 1 ? clientId : room.clients.get(clientId).pairId;
    }

    socket.on('data', function (data) {
        let client = room.clients.get(clientId);
        if (data + "" === "OK") {
            onOKReceived(room, client);
        } else {
            onDataReceived(room, client, data + "");
        }
    });
    socket.on('close', function (data) {
        if (rooms.has(roomId) && room.clients.has(clientId)) {
            let client = room.clients.get(clientId);
            console.log("Client[" + client.username + "]\t[Closed]");

            trySendData(room, room.clients.get(client.pairId), TCP_ERROR_CODE_PAIR_HAS_LEFT);

            rooms.delete(roomId);
        }
    });
}).listen(3000);

function onDataReceived(room, client, data) {
    console.log("Client[" + client.username + "]\tRoomStatus[" + room.status + "]\tClientStatus[" + client.status + "]\t[" + data + "]");
    switch (room.status) {
        case ROOM_INITIALIZED:
            dataReceivedOnRoomInitialized(room, client, data);
            break;
        case ROOM_GAME_STARTED:
            dataReceivedOnRoomGameStarted(room, client, data);
            break;
    }
}

function onOKReceived(room, client) {

    console.log("Client[" + client.username + "]\tRoomStatus[" + room.status + "]\tClientStatus[" + client.status + "]\t[OK Received]");

    switch (room.status) {
        case ROOM_INITIALIZED:
            if (client.status === CLIENT_USERNAME_TAKEN) {
                client.status = CLIENT_PAIR_USERNAME_SENT;
                if (room.clients.get(client.pairId).status === CLIENT_PAIR_USERNAME_SENT) {
                    // Send GameInfo
                    console.log("[GAME IS STARTING]");

                    setTimeout(() => {
                        room.clients.forEach((c, c_id) => {
                            if (room.whoseTurn === c_id) {
                                trySendData(room, c, TCP_STARTER_DATA_PREFIX + c.symbol);
                            } else {
                                trySendData(room, c, TCP_WAITER_DATA_PREFIX + c.symbol);
                            }
                        });
                    }, 1000);

                }
            } else if (client.status === CLIENT_PAIR_USERNAME_SENT) {
                client.status = CLIENT_START_INFO_SENT;

                if (room.clients.get(client.pairId).status === CLIENT_START_INFO_SENT) {

                    room.clients.get(room.whoseTurn).status = CLIENT_READY;
                    room.clients.get(room.clients.get(room.whoseTurn).pairId).status = CLIENT_BUSY;

                    room.status = ROOM_GAME_STARTED;
                }
            }
            break;
        case ROOM_GAME_STARTED:
            if (client.id !== room.whoseTurn) {
                client.status = CLIENT_READY;
                room.clients.get(client.pairId).status = CLIENT_BUSY;
                room.whoseTurn = client.id;
            }
            break;
    }
}

function dataReceivedOnRoomInitialized(room, client, data) {

    if (client.status === CLIENT_JOINED) {  // Username received.
        client.username = data;
        client.status = CLIENT_USERNAME_TAKEN;

        if (client.pairId !== null && room.clients.get(client.pairId).status === CLIENT_USERNAME_TAKEN) {
            trySendData(room, client, room.clients.get(client.pairId).username);
            trySendData(room, room.clients.get(client.pairId), client.username);
        }
    }
}

function dataReceivedOnRoomGameStarted(room, client, data) {

    if (client.id === room.whoseTurn) {
        let location = parseInt(data + "", 10);

        room.xoxTable.set(location, client.symbol);

        let status = gameStatus(room, location);

        if (status === 0) {
            trySendData(room, room.clients.get(client.pairId), TCP_GAME_DATA_PREFIX_BY_STATUS[status] + location);
        }
        else {
            console.log("Game Finished | Status : " + TCP_GAME_DATA_PREFIX_BY_STATUS[status]);
            room.xoxTable.clear();
            client.score++;
            trySendData(room, room.clients.get(client.pairId), TCP_GAME_DATA_PREFIX_BY_STATUS[status === 3 ? 3 : 2] + location);
            trySendData(room, client, TCP_GAME_DATA_PREFIX_BY_STATUS[status === 3 ? 3 : 1] + location + 'N');
        }
    }
}

function getAvailableRoomIndex(clientId) {

    let roomId = null;
    rooms.forEach((room, key) => {
        if (room.clients.size === 1) {
            roomId = key;
        }
    });

    if (roomId === null) {
        roomId = uuidv4();
        rooms.set(roomId, {
            status: ROOM_INITIALIZED,
            xoxTable: new Map(),
            whoseTurnIndex: randomBool(),
            whoseTurn: null,
            clients: new Map(),
            id: roomId
        });
    } else {
        rooms.get(roomId).clients.get(rooms.get(roomId).clients.keys().next().value).pairId = clientId;
    }

    return roomId;
}

function trySendData(room, client, data) {
    try {
        client.socket.write("[" + data + "]");
    } catch (e) {
        if (rooms.has(room.id)) {
            trySendData(room, room.clients.get(client.pairId), TCP_ERROR_CODE_PAIR_CONNECTION_LOST);
            rooms.delete(room.id);
        }
    }
}

function randomBool() {
    return Math.random() < 0.5 ? 0 : 1;
}

function uuidv4() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        let r = Math.random() * 16 | 0, v = c === 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}

function gameStatus(room, location) {

    let row = parseInt(location / 3);
    let col = parseInt(location % 3);
    console.log("row: " + row + "\tcol: " + col);

    let row_0 = row * 3;
    let col_0 = col;

    if (room.xoxTable.get(row_0) === room.xoxTable.get(row_0 + 1) && room.xoxTable.get(row_0) === room.xoxTable.get(row_0 + 2)) {
        return 1;
    }

    if (room.xoxTable.get(col_0) === room.xoxTable.get(col_0 + 3) && room.xoxTable.get(col_0) === room.xoxTable.get(col_0 + 6)) {
        return 1;
    }

    if (location % 2 === 0 && room.xoxTable.get(4) !== undefined) {
        if ((room.xoxTable.get(0) === room.xoxTable.get(4) && room.xoxTable.get(0) === room.xoxTable.get(8))
            ||
            (room.xoxTable.get(2) === room.xoxTable.get(4) && room.xoxTable.get(2) === room.xoxTable.get(6))) {
            return 1; // Win
        }
    }

    for (let i = 0; i < 9; i++) {
        if (room.xoxTable.get(i) === undefined) return 0;   // Continue
    }

    return 3; // Draw
}