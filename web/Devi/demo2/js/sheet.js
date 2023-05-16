var sheet;
var raw_sheet;

function isAlphaNumeric(str) {
var code, i, len;

for (i = 0, len = str.length; i < len; i++) {
    code = str.charCodeAt(i);
    if (!(code > 47 && code < 58) && // numeric (0-9)
        !(code > 64 && code < 91) && // upper alpha (A-Z)
        !(code > 96 && code < 123)) { // lower alpha (a-z)
    return false;
    }
}
return true;
};

function isAlpha(str) {
    var code, i, len;
    
    for (i = 0, len = str.length; i < len; i++) {
        code = str.charCodeAt(i);
        if (
            !(code > 64 && code < 91) && // upper alpha (A-Z)
            !(code > 96 && code < 123)) { // lower alpha (a-z)
        return false;
        }
    }
    return true;
    };

function isNumeric(str) {
    var code, i, len;
    
    for (i = 0, len = str.length; i < len; i++) {
        code = str.charCodeAt(i);
        if (!(code > 47 && code < 58) ) {
        return false;
        }
    }
    return true;
    };

function getSheet() 
{
    // var _A = "A"; 
    // var _B = "D";
    // var dupa = Math.abs(_A.charCodeAt(0) - _B.charCodeAt(0)) + 1;
    // console.log("dupa:  " + dupa);
    var toSend = 
	{
		"sin":   getCookie("sin")
	};

    postData('POST', urlRoot + '/sheet/1/get', toSend)
	.then((data) => {
        console.log(data);
        var result = {};
        raw_sheet = data['sheet'];
        console.log(data['sheet']);
		for(var cell in data['sheet'])
        {
            result[cell] = data['sheet'][cell];
            for(var d in data['data'])
            {
                result[cell] = result[cell].replace(d, data['data'][d])
            }
            if(result[cell][0] == '=') result[cell] = result[cell].substr(1);
            try{result[cell] = math.evaluate(result[cell]);} catch {;}
        }
        var pre = JSON.parse(JSON.stringify(result));
        console.log(pre);

        for(var _cell in result)
        {
            var resolve = (cell) =>
            {
                var txt = result[cell];
                if(!isNaN(txt)) txt;
                if(txt === undefined) return 0;
                
                var segment = "";
                //console.log("__>" + txt);
                for(var i = 0; i < txt.length; i++) 
                {
                    var char = txt[i];
                    if(!isAlphaNumeric(char) || i == txt.length - 1)
                    {
                        if(i == txt.length - 1) segment += char;
                        switch (segment) {
                            case "CELI":
                                var arg = "";
                                var flag = false;
                                i++;
                                for (i; (i < txt.length || (txt[i] == ")" && flag == false)); i++) {
                                    if(txt[i] == "(") flag = true;
                                    if(txt[i] == ")")
                                    {
                                        if(flag == true) flag = false;
                                        else break;
                                    }
                                    arg += txt[i];
                                }
                                var w = 0;
                                try{w = math.evaluate(arg);} catch {;}
                                result[cell] = result[cell].replace("CELI(" + arg +")", Math.ceil(w));
                                break;
                            case "FLOOR":
                                var arg = "";
                                var flag = false;
                                i++;
                                for (i; (i < txt.length || (txt[i] == ")" && flag == false)); i++) {
                                    if(txt[i] == "(") flag = true;
                                    if(txt[i] == ")")
                                    {
                                        if(flag == true) flag = false;
                                        else break;
                                    }
                                    arg += txt[i];
                                }
                                var w = 0;
                                try{w = math.evaluate(arg);} catch {;}
                                result[cell] = result[cell].replace("FLOOR(" + arg +")", Math.floor(w));
                                break;
                            case "ROUND":
                                var arg = "";
                                var flag = false;
                                i++;
                                for (i; (i < txt.length || (txt[i] == ")" && flag == false)); i++) {
                                    if(txt[i] == "(") flag = true;
                                    if(txt[i] == ")")
                                    {
                                        if(flag == true) flag = false;
                                        else break;
                                    }
                                    arg += txt[i];
                                }
                                var w = 0;
                                try{w = math.evaluate(arg);} catch {;}
                                result[cell] = result[cell].replace("ROUND(" + arg +")", Math.round(w));
                                break;
                            case "MAX":
                                var A = txt.substr(i + 1, txt.indexOf(":", i) - i - 1);
                                var _A = txt.substr(i + 1, 1);
                                var B = txt.substr(txt.indexOf(":", i) + 1, txt.indexOf(")", i) - txt.indexOf(":", i) - 1);
                                var _B = txt.substr(txt.indexOf(":", i) + 1, 1);
                                var first = Math.min(_A.charCodeAt(0), _B.charCodeAt(0));
                                var max = result[String.fromCharCode(first) + Number(A.substr(1))];
                                for (let ii = 0; ii < Math.abs(_A.charCodeAt(0) - _B.charCodeAt(0)) + 1; ii++) 
                                    for (let ij = Number(A.substr(1)); ij < Number(B.substr(1) + 1); ij++) 
                                        if(result[String.fromCharCode(first + ii) + ij] > max) max = result[String.fromCharCode(first + ii) + ij]

                                result[cell] = result[cell].replace("MAX" + txt.substr(i, txt.indexOf(")", i) - i + 1), max);
                                i = txt.indexOf(")", i);
                                break;
                            
                            case "MIN":
                                var A = txt.substr(i + 1, txt.indexOf(":", i) - i - 1);
                                var _A = txt.substr(i + 1, 1);
                                var B = txt.substr(txt.indexOf(":", i) + 1, txt.indexOf(")", i) - txt.indexOf(":", i) - 1);
                                var _B = txt.substr(txt.indexOf(":", i) + 1, 1);
                                var first = Math.min(_A.charCodeAt(0), _B.charCodeAt(0));
                                var min = result[String.fromCharCode(first) + Number(A.substr(1))];
                                for (let ii = 0; ii < Math.abs(_A.charCodeAt(0) - _B.charCodeAt(0)) + 1; ii++) 
                                    for (let ij = Number(A.substr(1)); ij < Number(B.substr(1) + 1); ij++) 
                                        if(result[String.fromCharCode(first + ii) + ij] < min) min = result[String.fromCharCode(first + ii) + ij]

                                result[cell] = result[cell].replace("MIN" + txt.substr(i, txt.indexOf(")", i) - i + 1), min);
                                i = txt.indexOf(")", i);
                                break;

                                case "SUM":
                                    var A = txt.substr(i + 1, txt.indexOf(":", i) - i - 1);
                                    var _A = txt.substr(i + 1, 1);
                                    var B = txt.substr(txt.indexOf(":", i) + 1, txt.indexOf(")", i) - txt.indexOf(":", i) - 1);
                                    var _B = txt.substr(txt.indexOf(":", i) + 1, 1);
                                    var first = Math.min(_A.charCodeAt(0), _B.charCodeAt(0));
                                    var sum = 0; 
                                    for (let ii = 0; ii < Math.abs(_A.charCodeAt(0) - _B.charCodeAt(0)) + 1; ii++) 
                                        for (let ij = Number(A.substr(1)); ij < Number(B.substr(1)) + 1; ij++) 
                                            if(!isNaN(result[String.fromCharCode(first + ii) + ij])) sum += result[String.fromCharCode(first + ii) + ij];

        
                                    result[cell] = result[cell].replace("SUM" + txt.substr(i, txt.indexOf(")", i) - i + 1), sum);
                                    i = txt.indexOf(")", i);
                                    break;

                                case "AVG":
                                    var A = txt.substr(i + 1, txt.indexOf(":", i) - i - 1);
                                    var _A = txt.substr(i + 1, 1);
                                    var B = txt.substr(txt.indexOf(":", i) + 1, txt.indexOf(")", i) - txt.indexOf(":", i) - 1);
                                    var _B = txt.substr(txt.indexOf(":", i) + 1, 1);
                                    var first = Math.min(_A.charCodeAt(0), _B.charCodeAt(0));
                                    var sum = 0; 
                                    var count = 0;
                                    for (let ii = 0; ii < Math.abs(_A.charCodeAt(0) - _B.charCodeAt(0)) + 1; ii++) 
                                        for (let ij = Number(A.substr(1)); ij < Number(B.substr(1)) + 1; ij++) 
                                            if(!isNaN(result[String.fromCharCode(first + ii) + ij]))
                                            {
                                                sum += result[String.fromCharCode(first + ii) + ij];
                                                count++;
                                            }

                                    result[cell] = result[cell].replace("AVG" + txt.substr(i, txt.indexOf(")", i) - i + 1), (sum / count));
                                    i = txt.indexOf(")", i);
                                    break;
                                case "COUNT":
                                    var A = txt.substr(i + 1, txt.indexOf(":", i) - i - 1);
                                    var _A = txt.substr(i + 1, 1);
                                    var B = txt.substr(txt.indexOf(":", i) + 1, txt.indexOf(")", i) - txt.indexOf(":", i) - 1);
                                    var _B = txt.substr(txt.indexOf(":", i) + 1, 1);
                                    var first = Math.min(_A.charCodeAt(0), _B.charCodeAt(0));
                                    var count = 0;
                                    for (let ii = 0; ii < Math.abs(_A.charCodeAt(0) - _B.charCodeAt(0)) + 1; ii++) 
                                        for (let ij = Number(A.substr(1)); ij < Number(B.substr(1)) + 1; ij++) 
                                            if(!isNaN(result[String.fromCharCode(first + ii) + ij]))
                                                count++;

                                    result[cell] = result[cell].replace("COUNT" + txt.substr(i, txt.indexOf(")", i) - i + 1), count);
                                    i = txt.indexOf(")", i);
                                    break;

                            default:
                                
                                if(!(segment != "" && isAlpha(segment.substr(0,1)) && isNumeric(segment.substr(1)))) continue;
                                result[cell] = result[cell].replace(segment, resolve(segment))
                                
                                //result[cell] = "!ERR";
                                break;
                        }
                        //console.log(segment);
                        segment = "";
                        continue;
                    }

                    try{result[cell] = math.evaluate(result[cell]);} catch {;}

                    segment += char;
                }

                try{result[cell] = math.evaluate(result[cell]);} catch {;}
                return result[cell];
            }
            resolve(_cell)
            //console.log(txt);

        }

        console.log(result);
        sheet = result;
        draw();
	})
    .catch((error) => {
        console.log(error);
       //logoutScript();
    });
}

function draw() {
    document.getElementById("tab").innerHTML = "";
    var content = "";
    for (let i = 0; i < 251; i++) {
        content += "<tr>";
        for(let j = 0; j < 27; j++)
        {
            if(i==0) content += (j == 0) ? "<td></td>" : "<th>" + String.fromCharCode(64 + j) + "</th>";
            else 
            {
                var value = sheet[String.fromCharCode(64 + j) + i];
                if(value === undefined) value = "";
                content += (j == 0) ? "<th>" + i + "</th>" : "<td onclick=\"afterClick('" + (String.fromCharCode(64 + j) + i) + "')\">" + value +"</td>";
            }
        }
        content += "</tr>";
    }
    document.getElementById("tab").innerHTML = content;
}

function updateSheet() {
    var coord = document.getElementById("coord").value;
    var exp = document.getElementById("exp").value;
    var toSend = 
	{
		"sin":   getCookie("sin"),
        "coord": coord,
        "value": exp
	};

    postData_nJSON('POST', urlRoot + '/sheet/1/update', toSend)
    .then((data) => {
        console.log(data.text());
        getSheet();
    })
    .catch((error)=> { console.log(error);});
}

function afterClick(coord) 
{
    document.getElementById("coord").value = coord;
    var val = raw_sheet[coord];
    if(val === undefined) val = "";
    document.getElementById("exp").value = val;
}