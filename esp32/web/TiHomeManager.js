function heatStateColor(m)
{
  let color = "white";
  if(m=="C") color="pink";
  else if(m=="E") color="lightgrey";
  else if(m=="F") color="lightblue";          
//  else if(m=="O") color="white";
  return color;
}

function heatStateSwitch(home,z,d,h,q)
{
  let m = home.heatzones[z].schedule[d][h*4+q]
  return color;
}

function heatZoneTable(home)
{
  const daystr=["lundi","mardi","mercredi","jeudi","vendredi","samedi","dimanche"];
  let tbl = document.createElement('table');
  tbl.style.width = '100px';
  tbl.style.border = '1px solid black';
  nzones = home.heatzones.length;  
  let header = tbl.createTHead();
  let tr = header.insertRow();
  let td = tr.insertCell();
  for(h=0;h<24;h++)
  {
    td = tr.insertCell();
    td.appendChild(document.createTextNode(h.toString().padStart(2,"0")+"h"));
    td.style.border = '1px solid black';
    td.style["text-align"] = 'center';
    td.setAttribute('colSpan', '4');
  }
  for (let z = 0; z < nzones; z++)
  {
    tr = tbl.insertRow();
    td = tr.insertCell();
    td.appendChild(document.createTextNode(`Zone ${z}`));
    td.style.border = '1px solid black';
    let color = heatStateColor( home.heatzones[z].state );
    //b.style.color = color;
    td.style["background-color"] = color;
    for (let d = 0; d < 7; d++)
    {
      tr = tbl.insertRow();
      td = tr.insertCell();
      td.appendChild(document.createTextNode(daystr[d]));
      td.style.border = '1px solid black';
      for(let h=0;h<24;h++)
      {
        for(q=0;q<4;q++)
        {
          td = tr.insertCell();
          let b = document.createElement('button');
          let color = heatStateColor( home.heatzones[z].schedule[d][h*4+q] );
          b.id = `z${z}d${d}h${h}q${q}`;
          b.innerHTML = "X";
          b.style.border = 0;
          b.style.color = color;
          b.style["background-color"] = color;
          b.onclick = '';
          td.appendChild(b);
        }
        td.style.border = 0;
      }
    }
  }
  return tbl;
}
let home = JSON.parse( document.getElementById("home_json").innerHTML );
document.body.appendChild( heatZoneTable(home) );
