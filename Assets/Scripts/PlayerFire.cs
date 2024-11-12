using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class PlayerFire : MonoBehaviour
{
    public GameObject bulletFactory; // �Ѿ� ������
    public Transform firePosition; // �Ѿ� �߻� ��ġ
    public float fireCooldown = 3.0f; // �߻� ��Ÿ�� (��)
    public AudioClip shootSound; // �߻� ����

    private float lastFireTime; // ������ �߻� �ð�
    private Text reloadText; // UI�� ǥ���� ������ �ؽ�Ʈ
    private AudioSource audioSource; // �߻� ���� ����� ���� AudioSource

    void Start()
    {
        // UI�� ǥ���� �ؽ�Ʈ ������Ʈ ��������
        reloadText = GameObject.Find("ReloadText").GetComponent<Text>();
        reloadText.text = ""; // �ʱ⿡�� �ؽ�Ʈ�� �����

        // AudioSource ������Ʈ�� �߰��ϰ� ����
        audioSource = gameObject.AddComponent<AudioSource>();
    }

    void Update()
    {
        // ������ �ð� �ؽ�Ʈ ������Ʈ
        UpdateReloadText();

        // �Ѿ� �߻�
        if (Input.GetButtonDown("Fire1"))
        {
            FireBullet();
        }
    }

    void FireBullet()
    {
        // ���� �ð��� ������ �߻� �ð��� ���̸� ���
        if (Time.time - lastFireTime >= fireCooldown)
        {
            // �Ѿ� ����
            GameObject bullet = Instantiate(bulletFactory, firePosition.position, firePosition.rotation);

            // �Ѿ��� �߻�� ���� ���� (��ž�� ����)
            if (Camera.main != null)
            {
                bullet.transform.forward = Camera.main.transform.forward;
            }

            // �߻� ���� ���
            if (shootSound != null && audioSource != null)
            {
                audioSource.PlayOneShot(shootSound);
            }

            // ������ �߻� �ð��� ���� �ð����� ����
            lastFireTime = Time.time;
        }
    }

    void UpdateReloadText()
    {
        // ������ ������ Ȯ���ϰ� �ؽ�Ʈ ������Ʈ
        if (Time.time - lastFireTime < fireCooldown)
        {
            reloadText.text = "������: " + Mathf.CeilToInt(fireCooldown - (Time.time - lastFireTime)) + "��";
        }
        else
        {
            reloadText.text = "���� �Ϸ�";
        }
    }
}
