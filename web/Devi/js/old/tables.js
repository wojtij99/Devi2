function getTables(_f) 
{
    var XHR = ajaxInit();
	if(XHR == null) return;
	
	var toSend = 
	{
		sin:   getCookie("sin")
	};
	var jsonString = JSON.stringify(toSend);

	XHR.open("POST","http://localhost:3001/tables",true);
	XHR.setRequestHeader("Content-Type", "application/json");
    
	XHR.onreadystatechange = function()
	{
		//alert(XHR.responseText);
		if(XHR.readyState == 4 && XHR.status != 200)
		{
			if(XHR.responseText == "Wrong SIN")
            {
                logoutScript();
            }
		}
		else if (XHR.readyState == 4 && XHR.status == 200)
		{
            _f(XHR.responseText);
            //return XHR.responseText;
		}
	};
	XHR.send(jsonString);
}

function getTablesScript(data) 
{
    data.split(",").forEach(element => {
        document.getElementById("tables").innerHTML = "<a href='?table=" + element + "'><div id='navElement'>" + element + "</div></a>" + document.getElementById("tables").innerHTML;
    });
}

function getTable(_f) 
{
    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const product = urlParams.get('table')
    if(product == null) return;
    var XHR = ajaxInit();
	if(XHR == null) return;
	
	var toSend = 
	{
		sin:   getCookie("sin")
	};
	var jsonString = JSON.stringify(toSend);

	XHR.open("POST","http://localhost:3001/tables/" + product + "/select/all",true);
	XHR.setRequestHeader("Content-Type", "application/json");
    
	XHR.onreadystatechange = function()
	{
		//alert(XHR.responseText);
		if(XHR.readyState == 4 && XHR.status != 200)
		{
			if(XHR.responseText == "Wrong SIN")
            {
                logoutScript();
            }
		}
		else if (XHR.readyState == 4 && XHR.status == 200)
		{
            _f(XHR.responseText);
            //return XHR.responseText;
		}
	};
	XHR.send(jsonString);
}

function getTableScript(data) 
{
    var json = data;
    var obj = JSON.parse(json);
    console.log(obj);
    var result = "";

    result += "<tr>";
    obj.Legend.forEach(element => {
        if(element != "ID")
            result += "<th>" + element +"</th>"
    });
    result += "</tr>";

    result += "<tr>";
    var i = 0;
    obj.row0.forEach(element => {
        if(i != 0)
            result += "<td><input type='text' value='" + element +"'></td>"
        i++;
    });
    result += "</tr>"; 

    /*obj.forEach(row => {
        result += "<tr>";
        row.forEach(element => {
            result += "<td>" + element +"</td>"
        });
        result += "</tr>"; 
    });*/

    document.getElementById("tab").innerHTML += result;
}