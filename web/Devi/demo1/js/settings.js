function getPage()
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('page')

    switch (product) {
        case "tables":
            getTables();
            break;
        case "table":
            getTable();
            break;
        default:
            getTables();
            break;
    }
}

function getTables() 
{
    var toSend = 
	{
		"sin":   getCookie("sin")
	};

    postData('POST', 'http://localhost:3001/tables', toSend)
	.then((data) => {
        var result = "<table><tr><th>Nazwa</th><th>Opcje</th></tr>";
        if(data['tables'] != "")
            data['tables'].split(",").forEach(element => {
                if(element[element.length - 1] == " ")
                    element = element.substring(0, element.length - 1);

                result += "<tr><td><a href='?page=table&table=" + element + "'><input type='text' value='" + element + "' id='t_" + element + "'></a></td>";
                result += "<td><button onclick=\"renameTable('" + element + "')\">RENAME</button><button onclick=\"dropTable('" + element + "')\">DROP</button></td></tr>";
            });

        result += "<tr><td><input type='text' id='addTable'></td><td><button onclick=\"addTable()\">ADD</button></td></tr>";


        document.getElementById("tab").innerHTML = result + "</table>";
	})
    .catch((error) => {
       logoutScript();
    });
}

function addTable()
{
    var toSend = 
	{
		"sin":   getCookie("sin"),
        "name": document.getElementById("addTable").value
	};
    
    postData('PUT', 'http://localhost:3001/tables/add', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
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
	
    postData('POST', 'http://localhost:3001/tables/' + product + '/select/all', toSend)
	.then((data) => {
        var result = "";
        var select = "";
        select += "<option value='INT'>INT</option>";
        select += "<option value='TEXT'>TEXT</option>";
        select += "<option value='DATETIME'>DATETIME</option>";
        select += "<option value='TIME'>TIME</option>";
        select += "<option value='DATE'>DATE</option>";
        select += "<option value='FLOAT'>FLOAT</option>";
        select += "<option value='BOOL'>BOOL</option>";

        result += "<table id='js_tab'><tr><th>Nazwa</th><th>Typ</th><th>Opcje</th></tr>";
        data['Legend'].forEach((element, i) => {
            if(element != "ID")
                result += "<tr><td><input type='text' value='" + element + "' id='cn_" + element + "'></td><td><select id='cp_" + element + "'>" + select +"<option value='"+data['Types'][i]+"' selected>"+data['Types'][i]+"</option></select></td><td><button onclick='alterColumn(\"" + element + "\")'>ALTER</button><button onclick='dropColumn(\"" + element + "\")'>DROP</button></td></tr>";
        });
        
        
        result += "<tr><td><input type='text' id='addName'></td><td><select id='addType'>" + select +"</select></td><td><button onclick=\"addColumn()\">ADD</button></td></tr></table>";

        document.getElementById("tab").innerHTML = result;
	})
    .catch((error) => {
        console.log(error);
        logoutScript();
    });
}

function addColumn()
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;
	
	var toSend = 
	{
        name: document.getElementById("addName").value,
        type: document.getElementById("addType").value,
		sin:   getCookie("sin")
	};
	
    postData('PUT', 'http://localhost:3001/tables/' + product + '/addColumn', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}

function alterColumn(_name)
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;
	
	var toSend = 
	{
        name: _name,
        newName: document.getElementById("cn_" + _name).value,
        newType: document.getElementById("cp_" + _name).value,
		sin:   getCookie("sin")
	};
	
    postData('POST', 'http://localhost:3001/tables/' + product + '/alter', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}

function dropColumn(_name)
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;
	
	var toSend = 
	{
        name: _name,
		sin:   getCookie("sin")
	};
	
    postData('DELETE', 'http://localhost:3001/tables/' + product + '/dropColumn', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}

function renameTable(_name)
{
	var toSend = 
	{
        name: document.getElementById("t_" + _name).value,
		sin:   getCookie("sin")
	};
	
    postData('POST', 'http://localhost:3001/tables/' + _name + '/rename', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}

function dropTable(_name)
{
	var toSend = 
	{
		sin:   getCookie("sin")
	};
	
    postData('DELETE', 'http://localhost:3001/tables/' + _name + '/dropTable', toSend)
	.then((data) => {
		console.log(data);
        location.reload();
	})
    .catch((error) => {
        console.log(error);
        location.reload();
    });
}

