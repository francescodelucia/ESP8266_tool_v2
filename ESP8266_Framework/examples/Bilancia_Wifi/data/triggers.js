$(document).ready(function($)
{
	var trigger = JSON.parse(GetTriggerData());
	var random_id = function  () 
	{
		var id_num = Math.random().toString(9).substr(2,3);
		var id_str = Math.random().toString(36).substr(2);
		return id_num + id_str;
	}
	function GetTriggerData(){
		var file = "/trigger.json";
		var rawFile = new XMLHttpRequest();
		rawFile.open("GET", file, false);
		var allText="[]"
		rawFile.onreadystatechange = function ()
		{
			if(rawFile.readyState === 4)
			{if(rawFile.status === 200 || rawFile.status == 0){allText = rawFile.responseText;}}
		} 
		rawFile.send(null);
		return allText;
    }
    function sendTriggerData(array_){
		navigator.sendBeacon("./storeTrig",JSON.stringify(array_));
		//trigger.push({value1:"[0-100]",value2:"[0-100]",action:"http://[link_action]"});
		trigger = JSON.parse(GetTriggerData());
		tbl="";
		$(document).find('.tbl_user_data').html(tbl);
		tbl = GetTable();
		$(document).find('.tbl_user_data').html(tbl);
		//console.log(trigger);
    }
	var tbl = '';
	function GetTable(){
		var ret="";
		ret +='<table class="table table-hover">'
			ret +='<thead>';
				ret +='<tr>';
				ret +='<th>Val Min</th>';
				ret +='<th>Val Max</th>';
				ret +='<th>Action</th>';
				ret +='<th>Options</th>';
				ret +='</tr>';
			ret +='</thead>';
			ret +='<tbody class="pippo">';
				$.each(trigger, function(index, val) 
				{
					var row_id = random_id();
					ret +='<tr class="r_data" row_id="'+row_id+'">';
						ret +='<td ><div class="row_data" edit_type="click" col_name="value1">'+val['value1']+'</div></td>';
						ret +='<td ><div class="row_data" edit_type="click" col_name="value2">'+val['value2']+'</div></td>';
						ret +='<td ><div class="row_data" edit_type="click" col_name="action">'+val['action']+'</div></td>';
						ret +='<td>';
							ret +='<span class="btn_edit" > <a href="#" class="btn btn-link " row_id="'+row_id+'" > Edit</a> </span>';
							ret +='<span class="btn_save"> <a href="#" class="btn btn-link"  row_id="'+row_id+'"> Save</a> | </span>';
							ret +='<span class="btn_cancel"> <a href="#" class="btn btn-link" row_id="'+row_id+'"> Cancel</a> | </span>';
							ret +='<span class="btn_delete"> <a href="#" class="btn btn-link" row_id="'+row_id+'"> Delete</a></span>';
						ret +='</td>';
					ret +='</tr>';
				});
			ret +='</tbody>';
		ret +='</table>'
		return ret;
	};
	tbl += GetTable();
	var btn = '';
	btn+='<button type="button" class="btn btn-primary">Trigger <span class="badge">+</span></button>';
	$(document).find('.tbl_user_data').html(tbl);
	$(document).find('.btn_add').html(btn);
	$(document).find('.btn_save').hide();
	$(document).find('.btn_cancel').hide(); 
	$(document).on('click', '.btn_add', function(event)
	{
		event.preventDefault(); 
		trigger.push({value1:"[0-100]",value2:"[0-100]",action:"http://[link_action]"});
		tbl="";
		$(document).find('.tbl_user_data').html(tbl);
		tbl = GetTable();
		$(document).find('.tbl_user_data').html(tbl);
		$(document).find('.btn_add').hide();
		//console.log(trigger);
	})	
	$(document).on('click', '.row_data', function(event) 
	{
		event.preventDefault(); 
		if($(this).attr('edit_type') == 'button'){return false;}
		$(this).closest('div').attr('contenteditable', 'true');
		$(this).addClass('bg-warning').css('padding','5px');
		$(this).focus();
	})	
	$(document).on('focusout', '.row_data', function(event) {
		event.preventDefault();
		if($(this).attr('edit_type') == 'button'){return false;}
		var row_id = $(this).closest('tr').attr('row_id'); 
		var row_div = $(this)
		.removeClass('bg-warning') //add bg css
		.css('padding','')
		var col_name = row_div.attr('col_name'); 
		var col_val = row_div.html(); 
		var arr = {};
		arr[col_name] = col_val;
		$.extend(arr, {row_id:row_id});
		$('.post_msg').html( '<pre class="bg-success">'+JSON.stringify(arr, null, 2) +'</pre>');
	})	
	$(document).on('click', '.btn_edit', function(event){
		event.preventDefault();
		var tbl_row = $(this).closest('tr');
		var row_id = tbl_row.attr('row_id');
		tbl_row.find('.btn_save').show();
		tbl_row.find('.btn_cancel').show();
		tbl_row.find('.btn_delete').show();
		tbl_row.find('.btn_edit').hide(); 
		tbl_row.find('.row_data')
		.attr('contenteditable', 'true')
		.attr('edit_type', 'button')
		.addClass('bg-warning')
		.css('padding','3px')
		tbl_row.find('.row_data').each(function(index, val){$(this).attr('original_entry', $(this).html());});
	});
	$(document).on('click', '.btn_cancel', function(event) {
		event.preventDefault();
		var tbl_row = $(this).closest('tr');
		var row_id = tbl_row.attr('row_id');
		tbl_row.find('.btn_save').hide();
		tbl_row.find('.btn_cancel').hide();
		tbl_row.find('.btn_delete').hide();
		tbl_row.find('.btn_edit').show();
		$(document).find('.btn_add').show();
		tbl_row.find('.row_data')
		.attr('edit_type', 'click')
		.removeClass('bg-warning')
		.css('padding','') 
		tbl_row.find('.row_data').each(function(index, val){$(this).html( $(this).attr('original_entry'));});  
	});
	$(document).on('click', '.btn_save', function(event) {
		event.preventDefault();
		$(document).find('.btn_add').show();
		var tbl_row = $(this).closest('tr');
		var row_id = tbl_row.attr('row_id');
		tbl_row.find('.btn_save').hide();
		tbl_row.find('.btn_cancel').hide();
		tbl_row.find('.btn_delete').hide();
		tbl_row.find('.btn_edit').show();
		//$(document).find('.btn_add').show();
		tbl_row.find('.row_data')
		.attr('edit_type', 'click')
		.removeClass('bg-warning')
		.css('padding','')
		
		var arr = {};
		var _arr = [] 
		tbl_row.find('.row_data').each(function(index, val)
		{
			var col_name = $(this).attr('col_name');  
			var col_val  =  $(this).html();
			arr[col_name] = col_val;
		});
		
		var k ={}
		$(document).find('.row_data').each(function(index, val)
		{
			var col_name = $(this).attr('col_name');  
			var col_val  =  $(this).html();
			k[col_name] = col_val.replace('<br>','');
			if(col_name=="action"){
				_arr.push(k)
				k = {};
			}
		});
		sendTriggerData(_arr);
	});
	$(document).on('click', '.btn_delete', function(event) {
		event.preventDefault();
		$(document).find('.btn_add').show();
		var tbl_row = $(this).closest('tr');
		var row_id = tbl_row.attr('row_id');
		tbl_row.find('.btn_save').hide();
		tbl_row.find('.btn_cancel').hide();
		tbl_row.find('.btn_delete').hide();
		tbl_row.find('.btn_edit').show();
		
		tbl_row.find('.row_data')
		.attr('edit_type', 'click')
		.removeClass('bg-warning')
		.css('padding','')
		var arr = {}; 
		var _arr = [];
		tbl_row.find('.row_data').each(function(index, val)
		{
			var col_name = $(this).attr('col_name');  
			var col_val  =  $(this).html();
			arr[col_name] = col_val;
		});
		var k ={}
		$(document).find('.row_data').each(function(index, val)
		{
			var col_name = $(this).attr('col_name');  
			var col_val  =  $(this).html();
			k[col_name] = col_val.replace('<br>','');
			if(col_name=="action"){
				_arr.push(k)
				k = {};
			}
		});
		for( var i = 0; i < _arr.length; i++){ 
			if(_arr[i]['action'] === arr['action'] &&  
				_arr[i]['value1'] === arr['value1'] && 
				_arr[i]['value2'] === arr['value2']){
				_arr.splice(i, 1); 
				i--;
			}
		}
		sendTriggerData(_arr);
	});
}); 
