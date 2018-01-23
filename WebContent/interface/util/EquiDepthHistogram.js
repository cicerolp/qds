class EquiDepthHistogram {

    constructor(container,widgetID,screenX,screenY,totalWidth,totalHeight){
	//set margins
	this.margins = {left:10,right:10,top:10,bottom:30}
	var renderingWidth = totalWidth - this.margins.left - this.margins.right;
	var legendWidth = Math.ceil(d3.min([renderingWidth *0.6,100]));
	var plotWidth = totalWidth - legendWidth;

	
	//
	this.renderingArea = {x:screenX,y:screenY,width:plotWidth,height:totalHeight};
	this.legendArea = {x:(screenX+plotWidth-10),y:screenY,width:legendWidth,height:totalHeight};
	
	//
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;

	//
	this.canvas = container
	    .append("g")
	    .attr("id","canvasEquiDepth_" + widgetID)
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left) + ", " + (this.renderingArea.y+this.margins.top) + ")");

	//
	this.plotGroup   = this.canvas.append("g").attr("id","plot");
	this.legendGroup = this.canvas.append("g").attr("id","legend");
	
	//
	this.xScale = d3.scaleBand()
	    .range([0,this.canvasWidth])
	    .paddingInner([0.3])
	    .paddingOuter([0.4]);
	this.xAxis  = d3.axisBottom(this.xScale);
	this.plotGroup
	    .append("g")
	    .attr("class","xAxis")
	    .attr("transform","translate(0," + (this.canvasHeight)  + ")");

	//
	this.yScale = d3.scaleLinear().range([this.canvasHeight,0]);
	this.yAxis  = d3.axisLeft(this.yScale);
	this.plotGroup
	    .append("g")
	    .attr("class","yAxis")
	    .attr("transform","translate("+(this.margins.left+5) + ",0)");

	//
	this.colorScale = d3.scaleQuantize()
	    .domain([0, 1])
	    .range(d3.schemeBlues[8]);
	// this.colorScale = d3.scaleLinear()
	//     .domain([0, 1])
	//     .range(['white','red']);
	
	//
	this.plotGroup.append("text").attr("id",widgetID + "_labelXAxis");
	this.plotGroup.append("text").attr("id",widgetID + "_labelYAxis");
	this.xLabel = "";
	this.yLabel = "";
	//
	this.updatePlot();
    }

    setXAxisLabel(xLabel){
	this.xLabel = xLabel;
    }
    
    setYAxisLabel(yLabel){
	this.yLabel = yLabel;
    }
    
    updateAxis(){
	var canvasWidth = this.canvasWidth;
	var canvasHeight = this.canvasHeight;
	
	//text label for the x axis
	this.xAxis(this.plotGroup.select(".xAxis"));
	this.plotGroup.select("#" + this.widgetID + "_labelXAxis")
	    .attr("x",(canvasWidth/2.0))
	    .attr("y",(canvasHeight + this.margins.top + 20))
	    .style("text-anchor", "middle")
	    .text(this.xLabel);
	//text label for the y axis
	this.yAxis(this.plotGroup.select(".yAxis"));
	this.plotGroup.select("#" + this.widgetID + "_labelYAxis")
	    .attr("transform", "rotate(-90)")
	    .attr("y", 0 - this.margins.left)
	    .attr("x",0 - (canvasHeight / 2))
	    .attr("dy", "1em")
	    .style("text-anchor", "middle")
	    .text(this.yLabel);
    }

    setData(newData){ //[{"label":label,"bins":[{"lower":0,"upper":0.1,"density":0.1}]}]
	//
	this.data = newData;
	//
	this.xScale.domain(this.data.map(d=>d.label));
	//
	var binEnds = this.data.map(entry=>[entry.bins[0].lower,entry.bins[entry.bins.length-1].upper]);
	var extentMin = d3.min(binEnds,d=>d[0]);
	var extentMax = d3.max(binEnds,d=>d[1]);

	//
	console.log("****",binEnds);
	this.yScale.domain([extentMin,extentMax]);
	//
	this.updatePlot();
    }
    
    updateBars(){
	if(this.data == undefined)
	    return;

	var histograms = this.plotGroup.selectAll(".histogram")
	    .data(this.data);

	//
	histograms.exit().remove();

	var allHistograms = histograms.enter()
	    .append("g")
	    .attr("class","histogram")
	    .merge(histograms);
	
	//
	var rectangles = allHistograms
	    .selectAll("rect")
	    .data(d=>d.bins.map(bin=>{bin.label = d.label;return bin}));

	//
	rectangles.exit().remove();
	rectangles
	    .enter()
	    .append("rect")
	    .merge(rectangles)
	    .attr("x",(d=>this.xScale(d.label)).bind(this))
	    .attr("y",(d=>this.yScale(d.upper)).bind(this))
	    .attr("width",(d=>(this.xScale.bandwidth())).bind(this))
	    .attr("height",(d=>(this.yScale(d.lower)-this.yScale(d.upper))).bind(this))
	    .attr("stroke","black")
	    .attr("fill",(d=>this.colorScale(d.density)).bind(this));
    }

    updateLegend(){
	var rectHeight = 14;
	var rectWidth = 15;
	var ySlack = 2;
	var xSlack = 3;
	var numrects = this.colorScale.range().length;
	var legendTotalHeight = (ySlack + rectHeight)*numrects + ySlack;
	var binWidth = 1.0/numrects;
	
	//
	this.legendGroup
	    .selectAll(".background")
	    .data([1])
	    .enter()
	    .append("rect")
	    .style("fill","none")
	    .attr("stroke","black")
	    .attr("x",this.legendArea.x)
	    .attr("y",0)
	    .attr("width",this.legendArea.width)
	    .attr("height",legendTotalHeight);

	//
	var legendItens = this.legendGroup
	    .selectAll(".item")
	    .data(this.colorScale.range());

	legendItens.exit().remove();
	legendItens
	    .enter()
	    .append("rect")
	    .merge(legendItens)
	    .attr("fill",d=>d)
	    .attr("stroke","black")
	    .attr("x",this.legendArea.x + xSlack)
	    .attr("y",(d,i)=>ySlack+i*(ySlack+rectHeight)) 
	    .attr("width",rectWidth)
	    .attr("height",rectHeight);

	//
	var labels = this.legendGroup
	    .selectAll("text")
	    .data(this.colorScale.range());
	labels.exit().remove();
	labels.enter()
	    .append("text")
	    .merge(labels)
	    .attr("alignment-baseline","middle")
	    .attr("y",function(d,i){return 0.5*rectHeight+ySlack+(rectHeight + ySlack)*i})
	    .attr("x",(function(){return (this.legendArea.x+2*xSlack + rectWidth)}).bind(this))
	    .attr("alignment-baseline","middle")
	    .text((d,i)=>(i*binWidth).toFixed(2) + " -- " + ((i+1)*binWidth).toFixed(2));
	    
	
	//
	// var rects = this.legendGroup.selectAll("rect").data(colorScale.range());
	// rects.exit().remove();
	// rects.enter()
	//     .append("rect")
	//     .merge(rects)
	//     .attr("y",function(d,i){return (rectHeight + ySlack)*i;})
	//     .attr("width",rectWidth)
	//     .attr("height",rectHeight)
	//     .attr("fill",d=>d)
	//     .style("stroke-width","1")
	//     .style("stroke","black");
    }
    
    updatePlot(){
	this.updateAxis();
	this.updateLegend();
	this.updateBars();
    }
}

