using Google.Protobuf.Protocol;
using Protocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Define;

public class MonsterController : CreatureController
{
    Coroutine _coSkill;

    protected override void Init()
    {
        base.Init();
    }

    protected override void UpdateIsIdle()
    {
        base.UpdateIsIdle();
    }

    public override void OnDamaged()
    {
        Debug.Log("Monster Hit !");
        //Managers.Object.Remove(Id);
        //Managers.Resource.Destroy(gameObject);
    }

    public override void UseSkill(int skillId)
    {
        if (skillId == 1)
        {
            State = CreatureState.Skill;
        }
        else if (skillId == 2)
        {
            State = CreatureState.Skill;
        }
    }
}
