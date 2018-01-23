class TimeSeriesWidget{

    constructor(container,ID){
	//set margins
	this.renderingArea = {x:screenX,y:screenY,width:totalWidth,height:totalHeight};
	this.margins = {left:50,right:10,top:10,bottom:30}
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;
	this.baseTime = baseTime;
	//
	this.data = [];	
	//
	this.canvas = container
	    .append("g")
	    .attr("id","line_" + widgetID)
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left) + ", " + (this.renderingArea.y+this.margins.top) + ")");
	
	//
	this.xScale = d3.scaleTime().range([0,this.canvasWidth]);
	this.xAxis  = d3.axisBottom(this.xScale);
	this.canvas
	    .append("g")
	    .attr("class","xAxis")
	    .attr("transform","translate(0," + this.canvasHeight  + ")");

	//
	this.yScale = d3.scaleLinear().range([this.canvasHeight,0]);
	this.yAxis  = d3.axisLeft(this.yScale)
	    .tickFormat(d3.format(".2s"));
	this.canvas
	    .append("g")
	    .attr("class","yAxis");

	//
	var brushGroup = this.canvas.append("g")
	    .attr("class","brushGroup");
	var myBrush = d3.brushX();
	brushGroup.call(myBrush);

	//
	this.canvas.append("text").attr("id",widgetID + "_labelXAxis");
	this.canvas.append("text").attr("id",widgetID + "_labelYAxis");
	this.xLabel = "Time";
	this.yLabel = "";
	//
	this.updatePlot();
    }

    setTimeSelectionCallBack(myCallback,moveCallback){
	var brushGroup = this.canvas.select(".brushGroup");
	var myBrush = d3.brushX();
	var scale = this.xScale;
	if(moveCallback){
	myBrush.on("brush", function(){
	    if(d3.event.selection == null)
		    myCallback(null)
		else{
		    var limits = d3.event.selection.map(d=>scale.invert(d));
		    limits[0] = Math.floor(limits[0]);
		    limits[1] = Math.ceil(limits[1]);
		    moveCallback({"widgetID":this.widgetID,"constraints":limits});
		}
	});
	}
	if(myCallback){
	    myBrush.on("end", (function(){
		//console.log("brush");
		if(d3.event.selection == null)
		    myCallback(null)
		else{
		    var limits = d3.event.selection.map(d=>scale.invert(d));
		    limits[0] = Math.floor(limits[0]);
		    limits[1] = Math.ceil(limits[1]);
		    myCallback({"widgetID":this.widgetID,"constraints":limits});
		}
	    }).bind(this));
	}
	
	brushGroup.call(myBrush);
    }
    
    setData(newData,colors){
	//
	this.data = newData.map(function(d,i){
	    return {"curve":d,"color":colors[i]}
	});
	this.updatePlot();
    }

    setYLabel(yLabel){
	this.yLabel = yLabel;	
    }
    
    addTimeSeries(curve,color){
	//
	//debugger
	console.log("curve",color,curve.map(d=>d[1]).reduce(function(valorAnterior, valorAtual, indice, array) {
	    return valorAnterior + (+valorAtual);
	}));
	//
	this.data.push({"curve":curve,"color":color});
	this.updatePlot();
    }

    clearPlot(){
	this.setData([],[]);
    }
    
    updateAxis(){
	var canvasWidth = this.canvasWidth;
	var canvasHeight = this.canvasHeight;
	
	//text label for the x axis
	this.xAxis(this.canvas.select(".xAxis"));
	this.canvas.select("#" + this.widgetID + "_labelXAxis")
	    .attr("x",(canvasWidth/2.0))
	    .attr("y",(canvasHeight + this.margins.top + 20))
	    .style("text-anchor", "middle")
	    .text(this.xLabel);
	//text label for the y axis
	this.yAxis(this.canvas.select(".yAxis"));
	this.canvas.select("#" + this.widgetID + "_labelYAxis")
	    .attr("transform", "rotate(-90)")
	    .attr("y", 0 - this.margins.left)
	    .attr("x",0 - (canvasHeight / 2))
	    .attr("dy", "1em")
	    .style("text-anchor", "middle")
	    .text(this.yLabel);
    }

    updateLines(){
	var lines = this.canvas
	    .selectAll(".curve")
	    .data(this.data)
	//
	lines.exit().remove();
	//
	var xScale = this.xScale;
	var yScale = this.yScale;
	var line = d3.line()
	    .x(d => xScale(d[0]) )
	    .y(d => yScale(d[1]) );
	lines.enter()
	    .append("path")
	    .attr("class","curve")
	    .merge(lines)
	    .attr("d",function(d){
		return line(d.curve);
	    })
	    .attr("fill","none")
	    .style("stroke",d=>d.color);
    }
    
    updatePlot(){
	//
	var xExtents = [];
	var yExtents = [];
	this.data.forEach(function(curve){
	    var d = curve.curve;
	    //
	    var lineXExtent = d3.extent(d,d=>d[0]);
	    xExtents.push(lineXExtent[0]);
	    xExtents.push(lineXExtent[1]);
	    //
	    var lineYExtent = d3.extent(d,d=>d[1]);
	    yExtents.push(lineYExtent[0]);
	    yExtents.push(lineYExtent[1]);
	});

	//
	var xExtent = d3.extent(xExtents);
	//xExtent = [new Date("2017-01-01"), new Date("2017-12-31")]
	this.xScale.domain( xExtent );
	this.xAxis.scale(this.xScale);
	//
	var yExtent = d3.extent(yExtents);
	if(yExtent[0] == yExtent[1]){
	    yExtent[0] -= (0.1*yExtent[0]);
	    yExtent[1] += (0.1*yExtent[1]);
	}
	this.yScale.domain( yExtent );
	this.yAxis.scale(this.yScale);
	//
	this.updateAxis()
	//points
	this.updateLines()
    }
}
