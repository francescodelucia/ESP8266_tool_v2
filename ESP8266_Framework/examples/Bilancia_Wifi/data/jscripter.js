var conf = JSON.parse(readTextFile('/config.json', function(text){return text;}));
var aSSID; 
try{aSSID=JSON.parse(readTextFile('/scanData.json', function(text){return text;})).nets;}catch(err){}

function readTextFile(file)
{
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    var allText=""
    rawFile.onreadystatechange = function ()
    {
        if(rawFile.readyState === 4)
        {
            if(rawFile.status === 200 || rawFile.status == 0)
            {
                allText = rawFile.responseText;                
            }
        }
    }
    rawFile.send(null);
    return allText;
}

function ScanWIFI(){
	selectCS = document.getElementById('CLIENT_SSID');
	selectCS.disabled=true;
	var ret = readTextFile('/scanData');
	console.log(ret);
	var aSSID = JSON.parse(ret).nets;
	var currentSSID =conf.CLIENT_SSID;
	try{
		if(aSSID.indexOf(currentSSID) === -1){aSSID.push(currentSSID);}
		while (selectCS.length > 0) {                
			selectCS.remove(0);
		}       
		for( _SSID in aSSID ){
			selectCS.add(new Option(aSSID[_SSID]));
		};
		selectCS.add(new Option(currentSSID));
		selectCS.value = currentSSID;
	}catch(err){}
	selectCS.disabled=false;
}
function LoadData(){
	var aLTZ =['+12','+11','+10','+09','+08','+07','+06','+05','+04','+03','+02','+01','00','-01','-02','-03','-04','-05','-06','-07','-08','-09','-10','-11','-12'];
	var aNTP =['On','Off'];
	var currentSSID =conf.CLIENT_SSID;
	var currentTZ=conf.NTP_TIME_ZONE;
	var currentNTP_SEL = conf.NTP_ON;
	try{
		if(aSSID.indexOf(currentSSID) === -1){aSSID.push(currentSSID);}
		while (selectCS.length > 0) {                
			selectCS.remove(0);
		}       
		for( _SSID in aSSID ){
			selectCS.add(new Option(aSSID[_SSID]));
		};
		selectCS.add(new Option(currentSSID));
		selectCS.value = currentSSID;
	}catch(err){}
	
	
	selectCS = document.getElementById('CLIENT_SSID');
	selectCS.add(new Option(currentSSID));
	selectCS.value = currentSSID;

	
	var inputs = Array.prototype.slice.call(document.querySelectorAll('form input'));

	Object.keys(conf).map(function (dataItem){
			inputs.map(function (inputItem){
					return (inputItem.name === dataItem) ? (inputItem.value = conf[dataItem]):false;
			});
	});
	selectCS = document.getElementById('CLIENT_SSID');
	/*while (selectCS.length > 0) {                
        selectCS.remove(0);
    }
    	for( _SSID in aSSID ){
		selectCS.add(new Option(aSSID[_SSID]));
	};*/
	selectCS.add(new Option(currentSSID));
	selectCS.value = currentSSID;

	selectNTP = document.getElementById('NTP_TIME_ZONE');
	
	for( _LTZ in aLTZ ){
		selectNTP.add( new Option( aLTZ[_LTZ]));
	};
	selectNTP.value = currentTZ;

	selectNTP_CB = document.getElementById('NTP_ON');
	for( _NTP in aNTP ){
		selectNTP_CB.add( new Option( aNTP[_NTP]));
	};
	selectNTP_CB.value = currentNTP_SEL;
}
function LoadDefaultData(){
	LoadData();
}
function restartPage(){
	location.reload();
}
function formSerialize(formElement) {
	const values = {};
	const inputs = formElement.elements;
	for (let i = 0; i < inputs.length; i++) {
		values[inputs[i].name] = inputs[i].value;
	}
	return values;
}
function sendData(){
	const r = formSerialize(document.querySelector('form'));
	navigator.sendBeacon("./store", JSON.stringify(r));
	setTimeout(restartPage, 1000);
}
LoadData();
document.getElementById('body').style.opacity='100';
document.getElementById('msg').style.opacity='0';
