// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import { A, AT } from '../union-underlying-type/a.js';
import { B, BT } from '../union-underlying-type/b.js';
import { C, CT } from '../union-underlying-type/c.js';


export enum ABC {
  NONE = 0,
  A = 555,
  B = 666,
  C = 777
}

export function unionToAbc(
  type: ABC,
  accessor: (obj:A|B|C) => A|B|C|null
): A|B|C|null {
  switch(ABC[type]) {
    case 'NONE': return null; 
    case 'A': return accessor(new A())! as A;
    case 'B': return accessor(new B())! as B;
    case 'C': return accessor(new C())! as C;
    default: return null;
  }
}

export function unionListToAbc(
  type: ABC, 
  accessor: (index: number, obj:A|B|C) => A|B|C|null, 
  index: number
): A|B|C|null {
  switch(ABC[type]) {
    case 'NONE': return null; 
    case 'A': return accessor(index, new A())! as A;
    case 'B': return accessor(index, new B())! as B;
    case 'C': return accessor(index, new C())! as C;
    default: return null;
  }
}
