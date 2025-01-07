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
    td.setAttribute('colSpan', '4');
  }
  for (let z = 0; z < nzones; z++)
  {
    tr = tbl.insertRow();
    td = tr.insertCell();
    td.appendChild(document.createTextNode(`Zone ${z} ${home.heatzones[z].state}`));
    td.style.border = '1px solid black';
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
          let color = "black";
          let m =  home.heatzones[z].schedule[d][h*4+q];
          if(m=="C") color="green";
          else if(m=="E") color="orange";
          else if(m=="F") color="red";          
          else if(m=="O") color="black";          
          b.innerHTML = "X";
          b.style.color = color;
          b.style["background-color"] = color;
          td.appendChild(b);
        }
        td.style.border = 0;
      }
    }
  }
  return tbl;
}
document.body.appendChild( heatZoneTable(home) );

