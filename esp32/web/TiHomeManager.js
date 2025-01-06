function heatZoneTable(home)
{
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
    td.appendChild(document.createTextNode(`${h}h`));
    td.style.border = '1px solid black';
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
      td.appendChild(document.createTextNode(`day ${d}`));
      td.style.border = '1px solid black';
      for(let h=0;h<24;h++)
      {
        td = tr.insertCell();
        for(q=0;q<4;q++)
        {
          let b = document.createElement('button');
          let m =  home.heatzones[z].schedule[d][h*4+q]
          b.innerHTML = m;
          td.appendChild(b);
        }
        td.style.border = 0;
      }
    }
  }
  return tbl;
}
document.body.appendChild( heatZoneTable(home) );

