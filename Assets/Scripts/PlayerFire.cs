using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class PlayerFire : MonoBehaviour
{
    public GameObject bulletFactory; // 총알 프리팹
    public Transform firePosition; // 총알 발사 위치
    public float fireCooldown = 3.0f; // 발사 쿨타임 (초)
    public AudioClip shootSound; // 발사 사운드

    private float lastFireTime; // 마지막 발사 시간
    private Text reloadText; // UI에 표시할 재장전 텍스트
    private AudioSource audioSource; // 발사 사운드 재생을 위한 AudioSource

    void Start()
    {
        // UI에 표시할 텍스트 컴포넌트 가져오기
        reloadText = GameObject.Find("ReloadText").GetComponent<Text>();
        reloadText.text = ""; // 초기에는 텍스트를 비워둠

        // AudioSource 컴포넌트를 추가하고 설정
        audioSource = gameObject.AddComponent<AudioSource>();
    }

    void Update()
    {
        // 재장전 시간 텍스트 업데이트
        UpdateReloadText();

        // 총알 발사
        if (Input.GetButtonDown("Fire1"))
        {
            FireBullet();
        }
    }

    void FireBullet()
    {
        // 현재 시간과 마지막 발사 시간의 차이를 계산
        if (Time.time - lastFireTime >= fireCooldown)
        {
            // 총알 생성
            GameObject bullet = Instantiate(bulletFactory, firePosition.position, firePosition.rotation);

            // 총알이 발사될 방향 설정 (포탑의 방향)
            if (Camera.main != null)
            {
                bullet.transform.forward = Camera.main.transform.forward;
            }

            // 발사 사운드 재생
            if (shootSound != null && audioSource != null)
            {
                audioSource.PlayOneShot(shootSound);
            }

            // 마지막 발사 시간을 현재 시간으로 갱신
            lastFireTime = Time.time;
        }
    }

    void UpdateReloadText()
    {
        // 재장전 중인지 확인하고 텍스트 업데이트
        if (Time.time - lastFireTime < fireCooldown)
        {
            reloadText.text = "재장전: " + Mathf.CeilToInt(fireCooldown - (Time.time - lastFireTime)) + "초";
        }
        else
        {
            reloadText.text = "장전 완료";
        }
    }
}
