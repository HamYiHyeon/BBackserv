using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletEnemy : MonoBehaviour
{
    public float speed = 10.0f;
    public GameObject impactEffect;

    void Start()
    {
        // 총알의 방향을 초기화합니다. transform.forward를 사용하여 생성된 방향으로 설정합니다.
        GetComponent<Rigidbody>().velocity = transform.forward * speed;
    }

    void OnCollisionEnter(Collision collision)
    {
        // 충돌 효과를 생성합니다.
        if (impactEffect != null)
        {
            Instantiate(impactEffect, transform.position, Quaternion.identity);
        }

        // 충돌한 게임 오브젝트가 "Player" 태그를 가진 경우 제거합니다.
        if (collision.gameObject.CompareTag("Player"))
        {
            Destroy(collision.gameObject);
        }

        // 총알을 제거합니다.
        Destroy(gameObject);
    }
}
