function getTables() 
{
    var toSend = 
	{
		"sin":   getCookie("sin")
	};

    postData('POST', 'http://localhost:3001/tables', toSend)
	.then((data) => {
		data['tables'].split(",").forEach(element => {
            document.getElementById("tables").innerHTML = "<a href='?table=" + element + "'><div id='navElement'>" + element + "</div></a>" + document.getElementById("tables").innerHTML;
        });
	})
    .catch((error) => {
        console.log(error);
       logoutScript();
    });
}

function getTable() 
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;
	
	var toSend = 
	{
		sin:   getCookie("sin")
	};

    var queruyStr = "";
    //TODO: Query string 
    //TODO: PAGE & LIMIT
	
    postData('POST', 'http://localhost:3001/tables/' + product + '/select/all?' + queruyStr, toSend)
	.then((data) => {
        console.log(data);
        var result ="";
        result += "<table id='js_tab'><tr id='rowLegend'>";
        data['Legend'].forEach((element, i) => {
            if(element != "ID")
                result += "<th id='L"+i+"'>" + element +"</th>";
        });
        result += "<th>opcje</th>";
        result += "</tr>";
        
        for (var row in data) 
        {
            if(row == "Legend") continue;
            if(row == "Types")  continue;

            result += "<tr id='" + row + "'>";
            var id = 0;
            data[row].forEach((element, i) => {
                if(i == 0)
                    id = element;
                else
                {
                    var type;
                    switch (data['Types'][i]) {
                        case "INT":     type = "number"; break;
                        case "TEXT":    type = "text"; break;
                        case "DATE":    type = "date"; break;
                        case "TIME":    type = "time"; break;
                        case "DATETIME":type = "datetime-local"; break;
                        case "FLOAT":   type = "number' step='any'"; break; //type = "number' step='0.01'";
                        case "BOOL":    type = "checkbox"; break;
                        default: break;
                    }
                    
                    if(type != "checkbox")
                        result += "<td><input type='" + type +"' value='" + element +"' id='" + data['Legend'][i] + id + "'></td>";
                    else
                    {
                        var ischeched = "";
                        if(element > 0) ischeched = "checked";
                        result += "<td><input type='checkbox' id='" + data['Legend'][i] + id + "' " + ischeched + "></td>";
                    }
                }
            });
            result += "<td><button onclick='updateData(" + id + ")'>UPDATE</button><button onclick='deleteData(" + id + ")'>DELETE</button></td>";
            result += "</tr>"; 
        }
        
        result += "<tr id='rowInsert'>";
        data['Legend'].forEach((element, i) => {
            var type;
            switch (data['Types'][i]) {
                case "INT":     type = "number"; break;
                case "TEXT":    type = "text"; break;
                case "DATE":    type = "date"; break;
                case "TIME":    type = "time"; break;
                case "DATETIME":type = "datetime-local"; break;
                case "FLOAT":   type = "number' step='any'"; break; //type = "number' step='0.01'";
                case "BOOL":    type = "checkbox"; break;
            
                default: break;
            }
            if(element != "ID")
                result += "<td><input type='" + type +"' id='insert_" + data['Legend'][i] + "'></td>";
        });
        result += "<td><button onclick='insertData()'>INSERT</button></td>";
        result += "</tr></table>";

        document.getElementById("tab").innerHTML += result;
	})
    .catch((error) => {
        console.log(error);
        logoutScript();
    });
}

function insertData() 
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;

    var toSend = 
	{
		sin:   getCookie("sin")
	};

    for (let i = 1; document.getElementById("L" + i) != null; i++) 
    {
        var name = document.getElementById("L" + i).innerHTML;
        if(document.getElementById("insert_" + name).type != "checkbox")
            toSend[name] = document.getElementById("insert_" + name).value;
        else 
            toSend[name] = document.getElementById("insert_" + name).checked ? "1" : "0";
    }
    console.log(toSend);

    postData('PUT', 'http://localhost:3001/tables/' + product + '/insert', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}

function updateData(id) 
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;

    var toSend = 
	{
		sin:   getCookie("sin")
	};

    for (let i = 1; document.getElementById("L" + i) != null; i++) 
    {
        var name = document.getElementById("L" + i).innerHTML;
        if(document.getElementById(name + id).type != "checkbox")
            toSend[name] = document.getElementById(name + id).value;
        else 
            toSend[name] = document.getElementById(name + id).checked ? "1" : "0";
    }
    console.log(toSend);

    
    postData('POST', 'http://localhost:3001/tables/' + product + '/update/' + id, toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
    
}

function deleteData(id) 
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;

    var toSend = 
	{
		sin:   getCookie("sin")
	};

    postData('DELETE', 'http://localhost:3001/tables/' + product + '/delete/' + id, toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}