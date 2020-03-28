
//uses charts.js library to make all sorts of graphs. also a serves as the launch point for other viewOptions like google maps and calendar
function displayViewOption(canvasId, records, viewOptionInfo, reportId) {
	if(!viewOptionInfo) {
		return;
	}
	//allow us to manipulate the graphing process with report form inputs (just put @variables in the JSON anywhere JSON will allow it)
	if(reportId) {
		var encodedViewOptions = genericParamReplace(JSON.stringify(viewOptionInfo), reportId, "@");
		//console.log(encodedViewOptions);
		viewOptionInfo =  JSON.parse(encodedViewOptions);
	}
	var type = viewOptionInfo.type;
	if(!type) {
		type = "line";
	}

	//map is handled by google maps, not charts.js, so catch that here:
	if(type == "map") {
		reportsDivSet("googleMapsDiv", reportId);
		return plotOnMap(globalMap, records, viewOptionInfo);
	} else if(type == "email") {
		reportsDivSet("searchResults-" + reportId, reportId);
		return sendEmailConfirm(records, viewOptionInfo, reportId);
	}	if(type == "calendar") {
		reportsDivSet("searchResults-" + reportId, reportId);
		return displayCalendar(records, viewOptionInfo, reportId);
	}

	reportsDivSet("reportVisualization", reportId);
	//in case we need to draw pie charts and no colors were supplied in the root of the viewOptionInfo:
	var strStockColors = "ff0099,ff9900,99ff99,9999ff,990099,00ff00,00ffff,ffff00,0000ff,999900,990000,ff0000,000000,ffffff,330000,990033";
	if(viewOptionInfo.colors) {
		strStockColors = viewOptionInfo.colors;
	}

	if(viewOptionInfo.colors) {
		strStockColors = viewOptionInfo.colors;
	}
	var stockColors = strStockColors.split(",");
	var defaultColor = stockColors[0];
	var noAxes = ["doughnut", "pie"];
	var plots = viewOptionInfo.plots;
	var caption = viewOptionInfo.caption;
	var dataSets = new Array();
	var labels = new Array();
	var labelsMade = false;
	var colorIndex = 0;
	
	var borderColor;
	if(plots) {
		for(var i=0; i<plots.length; i++) {
			var plot = plots[i];
			var colorsForPies = new Array();
			defaultWeight = 1;
			if(plot.lineWeight) {
				defaultWeight = plot.lineWeight;

			}
			if(!plot.lineWeight && viewOptionInfo.lineWeight) {
				defaultWeight = viewOptionInfo.lineWeight;
			}

			//console.log(plot);
			//narrow the data:
			var narrowedData = new Array();
			var colorToUse = new Array();
			if(records && records.length) {
				for(var j=0; j<records.length; j++) {
					var record = records[j];
					if(!viewOptionInfo.excludeValue || (viewOptionInfo.excludeValue && viewOptionInfo.excludeValue!=record[viewOptionInfo.labelColumn])) {
						//console.log(colorFix(thisColor));
						if(!labelsMade) {
							labels.push(record[viewOptionInfo.labelColumn]);
							var thisColor = stockColors[colorIndex];

							//console.log((thisColor));
							if(colorFix(thisColor) instanceof Array) {
								colorsForPies.push(colorFix(thisColor)[0]);
								
							} else {
								colorsForPies.push(colorFix(thisColor));
							}
							colorIndex++;
							if(colorIndex >= stockColors.length) {
								colorIndex = 0;
							}
						}
						if(plot.column) {
							narrowedData.push(parseFloat(record[plot.column]));
						}
					}
					//need a default in case color is not set
					var plotLineColor = stockColors[colorIndex];
					if(plot.color) {
						plotLineColor = plot.color;
					}

					plotLineColor = manipulateColor(plotLineColor, plot, record);

					if(!(plotLineColor instanceof Array)) {
						colorToUse.push(plotLineColor);
					}
			 
					//borderColor = colorToUse;
					if (viewOptionInfo.color) {
						borderColor = viewOptionInfo.color;
					}

				}
			}
			labelsMade = true;

			if (noAxes.indexOf(type) > -1) {
				console.log(colorsForPies);
				
				colorToUse = colorsForPies;
			}

			//colorToUse = colorsForPies;
			//console.log(colorFix(colorToUse));
			//
			borderColor = colorFix(borderColor);
			//console.log(colorFix(borderColor));
			if ((borderColor instanceof Array)) {
				borderColor = borderColor[0];
			}
			if(!borderColor) {
				borderColor = colorFix(defaultColor);
			}
			//borderColor = '#000000';
			//colorToUse = '#ff0000';
			//console.log(colorToUse[0]);
	 
			if(type == 'line' && colorToUse instanceof Array) {
				colorToUse = colorToUse[0];
			}
			//console.log(colorToUse);
			var plotShape = "circle";
			var radius = 4;
			if(plot["shape"]) {
				if(plot["shape"].type) {
					plotShape = plot["shape"].type;
				}
				if(plot["shape"].radius) {
					radius = plot["shape"].radius;
				}
				//allows the circle size to be changed by data
				if(plot["shape"].alternativeMagnitude) {
					//i also want to have a synonymn for alternativeMagnitude
					//harmonize how this works with gooogle maps
					radius = record[plot["shape"].alternativeMagnitude];
				}
		  
				radius = parseFloat(radius);
				if(plot.shape && plot.shape.alternativeMagnitudeDivisor) {
					radius = radius/plot.shape.alternativeMagnitudeDivisor;
				}

			}
			dataSets.push(
			{
	            label: plot.label,
	            backgroundColor: colorFix(colorToUse),
	            borderColor: borderColor,
	            data: narrowedData,
	            borderWidth: defaultWeight,
	            pointStyle : plotShape,
	            radius : radius,
	            fill: false,
	        });
		}
	}
	var scaleDisplay = true;
	if (noAxes.indexOf(type) > -1) {
   		scaleDisplay = false;
   	}

	var scales = {
        xAxes: [{
            display: scaleDisplay,
            scaleLabel: {
                display: true,
                labelString: viewOptionInfo.labelColumn
            }
        }],
        yAxes: [{
            display: scaleDisplay,
            scaleLabel: {
                display: true,
                labelString: 'Value'
            }
        }]
    }

    var config = {
        type: type,
        data: {
            labels: labels,
            datasets: dataSets
        },
        options: {
            responsive: true,
            title:{
                display:true,
                text: caption
            },
            tooltips: {
                mode: 'index',
                intersect: false,
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: scales
        }
    };
  
	// console.log(config);
	var canvas = false;
	if(document.getElementById(canvasId)) {
		canvas = document.getElementById(canvasId).getContext("2d");
	}
	//var canvas = document.getElementById(canvasId);
	if(canvas) {
		//canvas.style.display='block';
	} else {
		return;
	}
	window.graphics = null;
	window.graphics = new Chart(canvas, config);
	/*
	if(typeof(viewOptionInfo["width"]) != "undefined") {
		canvas.style.width= viewOptionInfo.width + "px";
	}
	if(viewOptionInfo.height) {
		canvas.style.height= viewOptionInfo.height + "px";
	}
	*/

	window.graphics.update();
}

//values can be a reportId if you want to read a live form
function genericParamReplace(stringIn, values, paramPrefix) {
	if(!paramPrefix) {
		paramPrefix = "@";
	}
	if((typeof values == "string")){
		var formIn;
		//values is actually a reportId, so read the live values off the form
		if(document.getElementById("reportForm-" + values)) {
			formIn = document.getElementById("reportForm-" + values).elements;
		} else if(document.getElementById("searchForm")) {
			formIn = document.getElementById("searchForm").elements;
		} else {
			return stringIn;
		}
		values = new Array();
		//console.log(formIn.length);
		for(var i=0; i<formIn.length; i++) {
			var element = formIn[i];
			//console.log(element);
			//if(formItem["name"]) {
				values[element.name] = decodeURIComponent(element.value);
			//}
		}
	}
	for(var key in values) {
		var value = values[key];
		if(key.trim() != "") {
			//console.log(value);
			//apparently there are errors in this process at times, so:
			try {
				var decodedUriComponent = decodeURIComponent(value);
				stringIn = stringIn.replace(paramPrefix + key, decodedUriComponent);
				//console.log(decodedJsonData);
			}
			catch (err) { 
				//dont bother
			}
 			
 		}
	}
	return stringIn;
}

function reportsDivSet(divToShow, reportId) {
	var allDivs = ["googleMapsDiv","reportVisualization", "searchResults-" . reportId, "searchResults"];
	//console.log(allDivs.length);
	if(reportId) {
		for(var i=0; i<allDivs.length; i++) {
			var divInQuestion = document.getElementById(allDivs[i]);
			if(divInQuestion) {
				//console.log(divInQuestion);
				if(allDivs[i] != divToShow) {
					//console.log(allDivs[i]);
					divInQuestion.style.display = 'none';
				} else {
					//console.log("ok:" + allDivs[i]);
					divInQuestion.style.display = 'block';
				}
			}
		}
	}
}


//returns either a string beginning with # or a possibly-acceptable color name based on the input. 
//returns an array if given one or if the colors are a string list 
function colorFix(strIn) {
	//console.log(strIn);
	if(strIn) {
		if((typeof strIn == "string")){
			strIn = strIn.replace('#', '').trim();
			if(strIn.indexOf(" ")) {
				strIn = strIn.split(" "); //handle below
			} else {
				if(isHexDec(strIn)) {
					return "#" + strIn.trim();
				} else {

					return strIn.trim();
				}
			}
		} 
		if (strIn instanceof Array) {
			for(var i=0; i<strIn.length; i++) {
				//console.log(strIn[i]);
				strIn[i] = strIn[i].replace('#', '');
				if(isHexDec(strIn)) {
					strIn[i] = "#" + strIn[i].trim();
				} else {

					strIn[i] = strIn[i].trim();
				}
			}
			return strIn;

		}
	}

}


function isHexDec(inVal) {
	var re = /[0-9A-Fa-f]{6}/g;
	if(re.test(inVal)) {
	    return true;
	} else {
	    return false;
	}
}


//does adjustments to colors based on plot parameters and record data
//this allows data to change colors dynamically in graphs and maps
function manipulateColor(plotColor, plot, record) {
	//do color effects from data if specified
	var darkenByDivisor = 1;
	var lightenByDivisor = 1;
	var saturateByDivisor = 1;
	var desaturateByDivisor = 1;

	if(plot.darkenByDivisor) {
		darkenByDivisor = plot.darkenByDivisor;
	}
	if(plot.lightenByDivisor) {
		lightenByDivisor = plot.lightenByDivisor;
	}
	if(plot.saturateByDivisor) {
		saturateByDivisor = plot.saturateByDivisor;
	}
	if(plot.desaturateByDivisor) {
		desaturateByDivisor = plot.desaturateByDivisor;
	}
	if(plot.darkenBy) {
		//console.log(plotLineColor + " " + tinycolor(plotLineColor).darken(30) + " "+ record[plot.darkenBy]);
		plotColor = tinycolor(plotColor.toString()).darken(parseInt(record[plot.darkenBy]/darkenByDivisor)).toHexString()
	}
	if(plot.lightenBy) {
		//console.log(plotLineColor + " " + tinycolor(plotLineColor).darken(30) + " "+ record[plot.darkenBy]);
		plotColor = tinycolor(plotColor.toString()).lighten(parseInt(record[plot.lightenBy]/lightenByDivisor)).toHexString()
	}
	if(plot.saturateBy) {
		//console.log(plotLineColor + " " + tinycolor(plotLineColor).darken(30) + " "+ record[plot.darkenBy]);
		plotColor = tinycolor(plotColor.toString()).saturate(parseInt(record[plot.saturateBy]/saturateByDivisor)).toHexString()
	}
	if(plot.desaturateBy) {
		//console.log(plotLineColor + " " + tinycolor(plotLineColor).darken(30) + " "+ record[plot.darkenBy]);
		plotColor = tinycolor(plotColor.toString()).desaturate(parseInt(record[plot.desaturateBy]/desaturateByDivisor)).toHexString()
	}
	return plotColor;
}

